#ifndef ORACON_LANG_PARSER_PARSER_H
#define ORACON_LANG_PARSER_PARSER_H

#include "oracon/lang/lexer/token.h"
#include "oracon/lang/ast/ast.h"
#include <vector>

namespace oracon {
namespace lang {

using core::String;
using core::usize;
using core::UniquePtr;

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    UniquePtr<Program> parse();

    bool hasError() const { return m_hasError; }
    const std::vector<String>& getErrors() const { return m_errors; }

private:
    const std::vector<Token>& m_tokens;
    usize m_current;
    bool m_hasError;
    std::vector<String> m_errors;

    // Error handling
    void addError(const String& message);
    void addError(const Token& token, const String& message);
    void synchronize();

    // Token navigation
    Token peek() const;
    Token previous() const;
    bool isAtEnd() const;
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(const std::vector<TokenType>& types);
    Token consume(TokenType type, const String& message);

    // Statement parsing
    UniquePtr<Stmt> declaration();
    UniquePtr<Stmt> varDeclaration();
    UniquePtr<Stmt> functionDeclaration(const String& kind);
    UniquePtr<Stmt> classDeclaration();
    UniquePtr<Stmt> statement();
    UniquePtr<Stmt> exprStatement();
    UniquePtr<Stmt> ifStatement();
    UniquePtr<Stmt> whileStatement();
    UniquePtr<Stmt> forStatement();
    UniquePtr<Stmt> returnStatement();
    UniquePtr<Stmt> breakStatement();
    UniquePtr<Stmt> continueStatement();
    UniquePtr<BlockStmt> blockStatement();

    // Expression parsing (by precedence)
    UniquePtr<Expr> expression();
    UniquePtr<Expr> assignment();
    UniquePtr<Expr> logicalOr();
    UniquePtr<Expr> logicalAnd();
    UniquePtr<Expr> equality();
    UniquePtr<Expr> comparison();
    UniquePtr<Expr> term();
    UniquePtr<Expr> factor();
    UniquePtr<Expr> unary();
    UniquePtr<Expr> power();
    UniquePtr<Expr> postfix();
    UniquePtr<Expr> primary();
};

} // namespace lang
} // namespace oracon

#endif // ORACON_LANG_PARSER_PARSER_H
