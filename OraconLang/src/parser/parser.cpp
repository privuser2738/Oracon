#include "oracon/lang/parser/parser.h"
#include <sstream>

namespace oracon {
namespace lang {

Parser::Parser(const std::vector<Token>& tokens)
    : m_tokens(tokens)
    , m_current(0)
    , m_hasError(false)
{}

UniquePtr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();

    while (!isAtEnd()) {
        try {
            auto stmt = declaration();
            if (stmt) {
                program->addStatement(std::move(stmt));
            }
        } catch (...) {
            synchronize();
        }
    }

    return program;
}

// ===== Error handling =====

void Parser::addError(const String& message) {
    m_errors.push_back(message);
    m_hasError = true;
}

void Parser::addError(const Token& token, const String& message) {
    std::ostringstream oss;
    oss << "Error at line " << token.getLocation().line << ", column " << token.getLocation().column
        << ": " << message;
    addError(oss.str());
}

void Parser::synchronize() {
    advance();

    while (!isAtEnd()) {
        if (previous().getType() == TokenType::SEMICOLON) return;

        switch (peek().getType()) {
            case TokenType::CLASS:
            case TokenType::FUNC:
            case TokenType::LET:
            case TokenType::CONST:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::RETURN:
                return;
            default:
                break;
        }

        advance();
    }
}

// ===== Token navigation =====

Token Parser::peek() const {
    return m_tokens[m_current];
}

Token Parser::previous() const {
    return m_tokens[m_current - 1];
}

bool Parser::isAtEnd() const {
    return peek().getType() == TokenType::EOF_TOKEN;
}

Token Parser::advance() {
    if (!isAtEnd()) m_current++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().getType() == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const String& message) {
    if (check(type)) return advance();
    addError(peek(), message);
    throw std::runtime_error(message);
}

// ===== Statement parsing =====

UniquePtr<Stmt> Parser::declaration() {
    try {
        if (match(TokenType::CLASS)) return classDeclaration();
        if (match(TokenType::FUNC)) return functionDeclaration("function");
        if (match({TokenType::LET, TokenType::CONST})) return varDeclaration();
        return statement();
    } catch (...) {
        synchronize();
        return nullptr;
    }
}

UniquePtr<Stmt> Parser::varDeclaration() {
    bool isConst = previous().getType() == TokenType::CONST;
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");

    UniquePtr<Expr> initializer = nullptr;
    if (match(TokenType::ASSIGN)) {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return std::make_unique<VarDeclStmt>(name, std::move(initializer), isConst);
}

UniquePtr<Stmt> Parser::functionDeclaration(const String& kind) {
    Token name = consume(TokenType::IDENTIFIER, "Expected " + kind + " name");
    consume(TokenType::LPAREN, "Expected '(' after " + kind + " name");

    std::vector<Token> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            if (parameters.size() >= 255) {
                addError(peek(), "Cannot have more than 255 parameters");
            }
            parameters.push_back(consume(TokenType::IDENTIFIER, "Expected parameter name"));
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "Expected ')' after parameters");
    consume(TokenType::LBRACE, "Expected '{' before " + kind + " body");
    auto body = blockStatement();

    return std::make_unique<FunctionStmt>(name, std::move(parameters), std::move(body));
}

UniquePtr<Stmt> Parser::classDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected class name");
    consume(TokenType::LBRACE, "Expected '{' before class body");

    std::vector<UniquePtr<FunctionStmt>> methods;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto method = functionDeclaration("method");
        methods.push_back(std::unique_ptr<FunctionStmt>(
            static_cast<FunctionStmt*>(method.release())));
    }

    consume(TokenType::RBRACE, "Expected '}' after class body");
    return std::make_unique<ClassStmt>(name, std::move(methods));
}

UniquePtr<Stmt> Parser::statement() {
    if (match(TokenType::IF)) return ifStatement();
    if (match(TokenType::WHILE)) return whileStatement();
    if (match(TokenType::FOR)) return forStatement();
    if (match(TokenType::RETURN)) return returnStatement();
    if (match(TokenType::BREAK)) return breakStatement();
    if (match(TokenType::CONTINUE)) return continueStatement();
    if (match(TokenType::LBRACE)) return blockStatement();
    return exprStatement();
}

UniquePtr<Stmt> Parser::exprStatement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    return std::make_unique<ExprStmt>(std::move(expr));
}

UniquePtr<Stmt> Parser::ifStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    auto condition = expression();
    consume(TokenType::RPAREN, "Expected ')' after if condition");

    auto thenBranch = statement();
    UniquePtr<Stmt> elseBranch = nullptr;
    if (match(TokenType::ELSE)) {
        elseBranch = statement();
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

UniquePtr<Stmt> Parser::whileStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    auto condition = expression();
    consume(TokenType::RPAREN, "Expected ')' after while condition");

    auto body = statement();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

UniquePtr<Stmt> Parser::forStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'for'");

    // Initializer
    UniquePtr<Stmt> initializer = nullptr;
    if (match(TokenType::SEMICOLON)) {
        initializer = nullptr;
    } else if (match({TokenType::LET, TokenType::CONST})) {
        initializer = varDeclaration();
    } else {
        initializer = exprStatement();
    }

    // Condition
    UniquePtr<Expr> condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after for condition");

    // Increment
    UniquePtr<Expr> increment = nullptr;
    if (!check(TokenType::RPAREN)) {
        increment = expression();
    }
    consume(TokenType::RPAREN, "Expected ')' after for clauses");

    auto body = statement();
    return std::make_unique<ForStmt>(std::move(initializer), std::move(condition),
                                      std::move(increment), std::move(body));
}

UniquePtr<Stmt> Parser::returnStatement() {
    Token keyword = previous();
    UniquePtr<Expr> value = nullptr;

    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after return value");
    return std::make_unique<ReturnStmt>(keyword, std::move(value));
}

UniquePtr<Stmt> Parser::breakStatement() {
    Token keyword = previous();
    consume(TokenType::SEMICOLON, "Expected ';' after 'break'");
    return std::make_unique<BreakStmt>(keyword);
}

UniquePtr<Stmt> Parser::continueStatement() {
    Token keyword = previous();
    consume(TokenType::SEMICOLON, "Expected ';' after 'continue'");
    return std::make_unique<ContinueStmt>(keyword);
}

UniquePtr<BlockStmt> Parser::blockStatement() {
    std::vector<UniquePtr<Stmt>> statements;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto stmt = declaration();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }

    consume(TokenType::RBRACE, "Expected '}' after block");
    return std::make_unique<BlockStmt>(std::move(statements));
}

// ===== Expression parsing =====

UniquePtr<Expr> Parser::expression() {
    return assignment();
}

UniquePtr<Expr> Parser::assignment() {
    auto expr = logicalOr();

    if (match(TokenType::ASSIGN)) {
        Token equals = previous();
        auto value = assignment();

        // Check if left side is a variable
        if (auto* var = dynamic_cast<VariableExpr*>(expr.get())) {
            Token name = var->getName();
            return std::make_unique<AssignmentExpr>(name, std::move(value));
        }

        addError(equals, "Invalid assignment target");
    }

    return expr;
}

UniquePtr<Expr> Parser::logicalOr() {
    auto expr = logicalAnd();

    while (match(TokenType::OR)) {
        Token op = previous();
        auto right = logicalAnd();
        expr = std::make_unique<LogicalExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

UniquePtr<Expr> Parser::logicalAnd() {
    auto expr = equality();

    while (match(TokenType::AND)) {
        Token op = previous();
        auto right = equality();
        expr = std::make_unique<LogicalExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

UniquePtr<Expr> Parser::equality() {
    auto expr = comparison();

    while (match({TokenType::EQUAL, TokenType::NOT_EQUAL})) {
        Token op = previous();
        auto right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

UniquePtr<Expr> Parser::comparison() {
    auto expr = term();

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL,
                  TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        auto right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

UniquePtr<Expr> Parser::term() {
    auto expr = factor();

    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

UniquePtr<Expr> Parser::factor() {
    auto expr = unary();

    while (match({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        Token op = previous();
        auto right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

UniquePtr<Expr> Parser::unary() {
    if (match({TokenType::NOT, TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        auto right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }

    return power();
}

UniquePtr<Expr> Parser::power() {
    auto expr = postfix();

    if (match(TokenType::POWER)) {
        Token op = previous();
        auto right = power(); // Right associative
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

UniquePtr<Expr> Parser::postfix() {
    auto expr = primary();

    while (true) {
        if (match(TokenType::LPAREN)) {
            // Function call
            Token paren = previous();
            std::vector<UniquePtr<Expr>> args;

            if (!check(TokenType::RPAREN)) {
                do {
                    if (args.size() >= 255) {
                        addError(peek(), "Cannot have more than 255 arguments");
                    }
                    args.push_back(expression());
                } while (match(TokenType::COMMA));
            }

            consume(TokenType::RPAREN, "Expected ')' after arguments");
            expr = std::make_unique<CallExpr>(std::move(expr), paren, std::move(args));

        } else if (match(TokenType::LBRACKET)) {
            // Array indexing
            auto index = expression();
            consume(TokenType::RBRACKET, "Expected ']' after index");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));

        } else if (match(TokenType::DOT)) {
            // Member access
            Token member = consume(TokenType::IDENTIFIER, "Expected property name after '.'");
            expr = std::make_unique<MemberExpr>(std::move(expr), member);

        } else {
            break;
        }
    }

    return expr;
}

UniquePtr<Expr> Parser::primary() {
    // Literals
    if (match(TokenType::TRUE)) return std::make_unique<LiteralExpr>(previous());
    if (match(TokenType::FALSE)) return std::make_unique<LiteralExpr>(previous());
    if (match(TokenType::NIL)) return std::make_unique<LiteralExpr>(previous());
    if (match(TokenType::INTEGER)) return std::make_unique<LiteralExpr>(previous());
    if (match(TokenType::FLOAT)) return std::make_unique<LiteralExpr>(previous());
    if (match(TokenType::STRING)) return std::make_unique<LiteralExpr>(previous());

    // Identifiers
    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<VariableExpr>(previous());
    }

    // Array literal
    if (match(TokenType::LBRACKET)) {
        std::vector<UniquePtr<Expr>> elements;

        if (!check(TokenType::RBRACKET)) {
            do {
                elements.push_back(expression());
            } while (match(TokenType::COMMA));
        }

        consume(TokenType::RBRACKET, "Expected ']' after array elements");
        return std::make_unique<ArrayExpr>(std::move(elements));
    }

    // Map literal
    if (match(TokenType::LBRACE)) {
        std::vector<MapExpr::KeyValuePair> pairs;

        if (!check(TokenType::RBRACE)) {
            do {
                // Expect key (identifier or string)
                if (!check(TokenType::IDENTIFIER) && !check(TokenType::STRING)) {
                    addError(peek(), "Expected identifier or string as map key");
                    throw std::runtime_error("Expected map key");
                }

                String key;
                if (match(TokenType::IDENTIFIER)) {
                    key = previous().getLexeme();
                } else if (match(TokenType::STRING)) {
                    key = previous().getLexeme();
                }

                consume(TokenType::COLON, "Expected ':' after map key");
                auto value = expression();

                pairs.push_back(std::make_pair(key, std::move(value)));
            } while (match(TokenType::COMMA));
        }

        consume(TokenType::RBRACE, "Expected '}' after map elements");
        return std::make_unique<MapExpr>(std::move(pairs));
    }

    // Grouping
    if (match(TokenType::LPAREN)) {
        auto expr = expression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }

    addError(peek(), "Expected expression");
    throw std::runtime_error("Expected expression");
}

} // namespace lang
} // namespace oracon
