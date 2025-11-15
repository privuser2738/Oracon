#ifndef ORACON_LANG_AST_AST_H
#define ORACON_LANG_AST_AST_H

#include "oracon/core/types.h"
#include "oracon/lang/lexer/token.h"
#include <vector>
#include <memory>

namespace oracon {
namespace lang {

using core::String;
using core::UniquePtr;

// Forward declarations
class Visitor;

// Base AST node
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual String toString() const = 0;
};

// Expression base class
class Expr : public ASTNode {
public:
    virtual ~Expr() = default;
};

// Statement base class
class Stmt : public ASTNode {
public:
    virtual ~Stmt() = default;
};

// ===== EXPRESSION NODES =====

// Literal expression (numbers, strings, booleans, nil)
class LiteralExpr : public Expr {
public:
    explicit LiteralExpr(const Token& token) : m_token(token) {}
    String toString() const override { return m_token.getLexeme(); }
    const Token& getToken() const { return m_token; }

private:
    Token m_token;
};

// Variable reference
class VariableExpr : public Expr {
public:
    explicit VariableExpr(const Token& name) : m_name(name) {}
    String toString() const override { return m_name.getLexeme(); }
    const Token& getName() const { return m_name; }

private:
    Token m_name;
};

// Unary expression (!, -, +)
class UnaryExpr : public Expr {
public:
    UnaryExpr(const Token& op, UniquePtr<Expr> operand)
        : m_operator(op), m_operand(std::move(operand)) {}

    String toString() const override;

    const Token& getOperator() const { return m_operator; }
    const Expr* getOperand() const { return m_operand.get(); }

private:
    Token m_operator;
    UniquePtr<Expr> m_operand;
};

// Binary expression (+, -, *, /, %, ==, !=, <, >, <=, >=, and, or, etc.)
class BinaryExpr : public Expr {
public:
    BinaryExpr(UniquePtr<Expr> left, const Token& op, UniquePtr<Expr> right)
        : m_left(std::move(left)), m_operator(op), m_right(std::move(right)) {}

    String toString() const override;

    const Expr* getLeft() const { return m_left.get(); }
    const Token& getOperator() const { return m_operator; }
    const Expr* getRight() const { return m_right.get(); }

private:
    UniquePtr<Expr> m_left;
    Token m_operator;
    UniquePtr<Expr> m_right;
};

// Grouping expression (parentheses)
class GroupingExpr : public Expr {
public:
    explicit GroupingExpr(UniquePtr<Expr> expr) : m_expr(std::move(expr)) {}
    String toString() const override;
    const Expr* getExpression() const { return m_expr.get(); }

private:
    UniquePtr<Expr> m_expr;
};

// Assignment expression (x = value)
class AssignmentExpr : public Expr {
public:
    AssignmentExpr(const Token& name, UniquePtr<Expr> value)
        : m_name(name), m_value(std::move(value)) {}

    String toString() const override;

    const Token& getName() const { return m_name; }
    const Expr* getValue() const { return m_value.get(); }

private:
    Token m_name;
    UniquePtr<Expr> m_value;
};

// Logical expression (and, or)
class LogicalExpr : public Expr {
public:
    LogicalExpr(UniquePtr<Expr> left, const Token& op, UniquePtr<Expr> right)
        : m_left(std::move(left)), m_operator(op), m_right(std::move(right)) {}

    String toString() const override;

    const Expr* getLeft() const { return m_left.get(); }
    const Token& getOperator() const { return m_operator; }
    const Expr* getRight() const { return m_right.get(); }

private:
    UniquePtr<Expr> m_left;
    Token m_operator;
    UniquePtr<Expr> m_right;
};

// Call expression (func(args))
class CallExpr : public Expr {
public:
    CallExpr(UniquePtr<Expr> callee, const Token& paren, std::vector<UniquePtr<Expr>> args)
        : m_callee(std::move(callee)), m_paren(paren), m_arguments(std::move(args)) {}

    String toString() const override;

    const Expr* getCallee() const { return m_callee.get(); }
    const Token& getParen() const { return m_paren; }
    const std::vector<UniquePtr<Expr>>& getArguments() const { return m_arguments; }

private:
    UniquePtr<Expr> m_callee;
    Token m_paren;
    std::vector<UniquePtr<Expr>> m_arguments;
};

// Array literal expression ([1, 2, 3])
class ArrayExpr : public Expr {
public:
    explicit ArrayExpr(std::vector<UniquePtr<Expr>> elements)
        : m_elements(std::move(elements)) {}

    String toString() const override;

    const std::vector<UniquePtr<Expr>>& getElements() const { return m_elements; }

private:
    std::vector<UniquePtr<Expr>> m_elements;
};

// Index expression (array[index])
class IndexExpr : public Expr {
public:
    IndexExpr(UniquePtr<Expr> object, UniquePtr<Expr> index)
        : m_object(std::move(object)), m_index(std::move(index)) {}

    String toString() const override;

    const Expr* getObject() const { return m_object.get(); }
    const Expr* getIndex() const { return m_index.get(); }

private:
    UniquePtr<Expr> m_object;
    UniquePtr<Expr> m_index;
};

// Member access expression (object.member)
class MemberExpr : public Expr {
public:
    MemberExpr(UniquePtr<Expr> object, const Token& member)
        : m_object(std::move(object)), m_member(member) {}

    String toString() const override;

    const Expr* getObject() const { return m_object.get(); }
    const Token& getMember() const { return m_member; }

private:
    UniquePtr<Expr> m_object;
    Token m_member;
};

// Map/dictionary expression ({key: value, ...})
class MapExpr : public Expr {
public:
    using KeyValuePair = std::pair<String, UniquePtr<Expr>>;

    explicit MapExpr(std::vector<KeyValuePair> pairs)
        : m_pairs(std::move(pairs)) {}

    String toString() const override;

    const std::vector<KeyValuePair>& getPairs() const { return m_pairs; }

private:
    std::vector<KeyValuePair> m_pairs;
};

// ===== STATEMENT NODES =====

// Expression statement
class ExprStmt : public Stmt {
public:
    explicit ExprStmt(UniquePtr<Expr> expr) : m_expr(std::move(expr)) {}
    String toString() const override;
    const Expr* getExpression() const { return m_expr.get(); }

private:
    UniquePtr<Expr> m_expr;
};

// Variable declaration (let x = value)
class VarDeclStmt : public Stmt {
public:
    VarDeclStmt(const Token& name, UniquePtr<Expr> initializer, bool isConst)
        : m_name(name), m_initializer(std::move(initializer)), m_isConst(isConst) {}

    String toString() const override;

    const Token& getName() const { return m_name; }
    const Expr* getInitializer() const { return m_initializer.get(); }
    bool isConst() const { return m_isConst; }

private:
    Token m_name;
    UniquePtr<Expr> m_initializer;
    bool m_isConst;
};

// Block statement ({ statements })
class BlockStmt : public Stmt {
public:
    explicit BlockStmt(std::vector<UniquePtr<Stmt>> statements)
        : m_statements(std::move(statements)) {}

    String toString() const override { return "Block"; }
    const std::vector<UniquePtr<Stmt>>& getStatements() const { return m_statements; }

private:
    std::vector<UniquePtr<Stmt>> m_statements;
};

// If statement
class IfStmt : public Stmt {
public:
    IfStmt(UniquePtr<Expr> condition, UniquePtr<Stmt> thenBranch, UniquePtr<Stmt> elseBranch)
        : m_condition(std::move(condition))
        , m_thenBranch(std::move(thenBranch))
        , m_elseBranch(std::move(elseBranch)) {}

    String toString() const override { return "If"; }

    const Expr* getCondition() const { return m_condition.get(); }
    const Stmt* getThenBranch() const { return m_thenBranch.get(); }
    const Stmt* getElseBranch() const { return m_elseBranch.get(); }

private:
    UniquePtr<Expr> m_condition;
    UniquePtr<Stmt> m_thenBranch;
    UniquePtr<Stmt> m_elseBranch;
};

// While statement
class WhileStmt : public Stmt {
public:
    WhileStmt(UniquePtr<Expr> condition, UniquePtr<Stmt> body)
        : m_condition(std::move(condition)), m_body(std::move(body)) {}

    String toString() const override { return "While"; }

    const Expr* getCondition() const { return m_condition.get(); }
    const Stmt* getBody() const { return m_body.get(); }

private:
    UniquePtr<Expr> m_condition;
    UniquePtr<Stmt> m_body;
};

// For statement
class ForStmt : public Stmt {
public:
    ForStmt(UniquePtr<Stmt> initializer, UniquePtr<Expr> condition,
            UniquePtr<Expr> increment, UniquePtr<Stmt> body)
        : m_initializer(std::move(initializer))
        , m_condition(std::move(condition))
        , m_increment(std::move(increment))
        , m_body(std::move(body)) {}

    String toString() const override { return "For"; }

    const Stmt* getInitializer() const { return m_initializer.get(); }
    const Expr* getCondition() const { return m_condition.get(); }
    const Expr* getIncrement() const { return m_increment.get(); }
    const Stmt* getBody() const { return m_body.get(); }

private:
    UniquePtr<Stmt> m_initializer;
    UniquePtr<Expr> m_condition;
    UniquePtr<Expr> m_increment;
    UniquePtr<Stmt> m_body;
};

// Return statement
class ReturnStmt : public Stmt {
public:
    explicit ReturnStmt(const Token& keyword, UniquePtr<Expr> value)
        : m_keyword(keyword), m_value(std::move(value)) {}

    String toString() const override { return "Return"; }

    const Token& getKeyword() const { return m_keyword; }
    const Expr* getValue() const { return m_value.get(); }

private:
    Token m_keyword;
    UniquePtr<Expr> m_value;
};

// Break statement
class BreakStmt : public Stmt {
public:
    explicit BreakStmt(const Token& keyword) : m_keyword(keyword) {}
    String toString() const override { return "Break"; }
    const Token& getKeyword() const { return m_keyword; }

private:
    Token m_keyword;
};

// Continue statement
class ContinueStmt : public Stmt {
public:
    explicit ContinueStmt(const Token& keyword) : m_keyword(keyword) {}
    String toString() const override { return "Continue"; }
    const Token& getKeyword() const { return m_keyword; }

private:
    Token m_keyword;
};

// Function declaration
class FunctionStmt : public Stmt {
public:
    FunctionStmt(const Token& name, std::vector<Token> params, UniquePtr<BlockStmt> body)
        : m_name(name), m_parameters(std::move(params)), m_body(std::move(body)) {}

    String toString() const override;

    const Token& getName() const { return m_name; }
    const std::vector<Token>& getParameters() const { return m_parameters; }
    const BlockStmt* getBody() const { return m_body.get(); }

private:
    Token m_name;
    std::vector<Token> m_parameters;
    UniquePtr<BlockStmt> m_body;
};

// Class declaration
class ClassStmt : public Stmt {
public:
    ClassStmt(const Token& name, std::vector<UniquePtr<FunctionStmt>> methods)
        : m_name(name), m_methods(std::move(methods)) {}

    String toString() const override;

    const Token& getName() const { return m_name; }
    const std::vector<UniquePtr<FunctionStmt>>& getMethods() const { return m_methods; }

private:
    Token m_name;
    std::vector<UniquePtr<FunctionStmt>> m_methods;
};

// Program (top-level)
class Program : public ASTNode {
public:
    void addStatement(UniquePtr<Stmt> stmt) {
        m_statements.push_back(std::move(stmt));
    }

    String toString() const override { return "Program"; }
    const std::vector<UniquePtr<Stmt>>& getStatements() const { return m_statements; }

private:
    std::vector<UniquePtr<Stmt>> m_statements;
};

} // namespace lang
} // namespace oracon

#endif // ORACON_LANG_AST_AST_H
