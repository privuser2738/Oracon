#ifndef ORACON_LANG_INTERPRETER_INTERPRETER_H
#define ORACON_LANG_INTERPRETER_INTERPRETER_H

#include "oracon/lang/ast/ast.h"
#include "oracon/lang/interpreter/environment.h"
#include "oracon/lang/interpreter/value.h"
#include <memory>
#include <vector>

namespace oracon {
namespace lang {

// Control flow exceptions
class ReturnException : public std::exception {
public:
    explicit ReturnException(const Value& value) : m_value(value) {}
    const Value& getValue() const { return m_value; }
private:
    Value m_value;
};

class BreakException : public std::exception {};
class ContinueException : public std::exception {};

class Interpreter {
public:
    Interpreter();

    void execute(const Program* program);
    bool hasError() const { return m_hasError; }
    const std::vector<String>& getErrors() const { return m_errors; }

    // Access to global environment (for scripting API)
    Environment& getGlobalEnv() { return m_globalEnv; }
    const Environment& getGlobalEnv() const { return m_globalEnv; }

    // Call a function by name from C++
    Value callFunction(const String& name, const std::vector<Value>& arguments);

private:
    Environment m_globalEnv;
    Environment* m_currentEnv;
    bool m_hasError;
    std::vector<String> m_errors;

    // Statement execution
    void executeStmt(const Stmt* stmt);
    void executeExprStmt(const ExprStmt* stmt);
    void executeVarDecl(const VarDeclStmt* stmt);
    void executeBlock(const BlockStmt* stmt, Environment* env);
    void executeIf(const IfStmt* stmt);
    void executeWhile(const WhileStmt* stmt);
    void executeFor(const ForStmt* stmt);
    void executeReturn(const ReturnStmt* stmt);
    void executeBreak(const BreakStmt* stmt);
    void executeContinue(const ContinueStmt* stmt);
    void executeFunctionDecl(const FunctionStmt* stmt);
    void executeClassDecl(const ClassStmt* stmt);

    // Expression evaluation
    Value evaluateExpr(const Expr* expr);
    Value evaluateLiteral(const LiteralExpr* expr);
    Value evaluateVariable(const VariableExpr* expr);
    Value evaluateUnary(const UnaryExpr* expr);
    Value evaluateBinary(const BinaryExpr* expr);
    Value evaluateGrouping(const GroupingExpr* expr);
    Value evaluateAssignment(const AssignmentExpr* expr);
    Value evaluateLogical(const LogicalExpr* expr);
    Value evaluateCall(const CallExpr* expr);
    Value evaluateArray(const ArrayExpr* expr);
    Value evaluateIndex(const IndexExpr* expr);
    Value evaluateMember(const MemberExpr* expr);
    Value evaluateMap(const MapExpr* expr);

    // Function calling
    Value callUserFunction(const FunctionType& function, const std::vector<Value>& arguments);

    // Error handling
    void runtimeError(const String& message);

    // Built-in functions
    void defineBuiltins();
};

} // namespace lang
} // namespace oracon

#endif // ORACON_LANG_INTERPRETER_INTERPRETER_H
