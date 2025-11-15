#include "oracon/lang/interpreter/interpreter.h"
#include "oracon/lang/lexer/token.h"
#include "oracon/core/logger.h"
#include <iostream>
#include <cmath>

namespace oracon {
namespace lang {

Interpreter::Interpreter()
    : m_currentEnv(&m_globalEnv)
    , m_hasError(false)
{
    defineBuiltins();
}

void Interpreter::execute(const Program* program) {
    try {
        for (const auto& stmt : program->getStatements()) {
            executeStmt(stmt.get());
        }
    } catch (const std::exception& e) {
        runtimeError(e.what());
    }
}

void Interpreter::defineBuiltins() {
    // print() - prints values to stdout
    auto printFn = std::make_shared<Function>("print", 1, [](const std::vector<Value>& args) -> Value {
        if (!args.empty()) {
            std::cout << args[0].toString() << std::endl;
        }
        return Value();
    });
    m_globalEnv.define("print", Value(printFn));

    // len() - returns length of arrays/maps/strings
    auto lenFn = std::make_shared<Function>("len", 1, [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value(static_cast<i64>(0));

        const Value& val = args[0];
        if (val.isArray()) {
            return Value(static_cast<i64>(val.arraySize()));
        } else if (val.isMap()) {
            return Value(static_cast<i64>(val.mapSize()));
        } else if (val.isString()) {
            return Value(static_cast<i64>(val.asString().length()));
        }
        return Value(static_cast<i64>(0));
    });
    m_globalEnv.define("len", Value(lenFn));

    // type() - returns type of value as string
    auto typeFn = std::make_shared<Function>("type", 1, [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value(String("nil"));

        const Value& val = args[0];
        if (val.isNil()) return Value(String("nil"));
        if (val.isBool()) return Value(String("boolean"));
        if (val.isInteger()) return Value(String("integer"));
        if (val.isFloat()) return Value(String("float"));
        if (val.isString()) return Value(String("string"));
        if (val.isArray()) return Value(String("array"));
        if (val.isMap()) return Value(String("map"));
        if (val.isFunction()) return Value(String("function"));
        return Value(String("unknown"));
    });
    m_globalEnv.define("type", Value(typeFn));
}

void Interpreter::runtimeError(const String& message) {
    m_hasError = true;
    m_errors.push_back(message);
    ORACON_LOG_ERROR("Runtime error: ", message);
}

// ===== Statement Execution =====

void Interpreter::executeStmt(const Stmt* stmt) {
    if (auto* exprStmt = dynamic_cast<const ExprStmt*>(stmt)) {
        executeExprStmt(exprStmt);
    } else if (auto* varDecl = dynamic_cast<const VarDeclStmt*>(stmt)) {
        executeVarDecl(varDecl);
    } else if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
        Environment blockEnv(m_currentEnv);
        executeBlock(block, &blockEnv);
    } else if (auto* ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
        executeIf(ifStmt);
    } else if (auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt)) {
        executeWhile(whileStmt);
    } else if (auto* forStmt = dynamic_cast<const ForStmt*>(stmt)) {
        executeFor(forStmt);
    } else if (auto* returnStmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        executeReturn(returnStmt);
    } else if (auto* breakStmt = dynamic_cast<const BreakStmt*>(stmt)) {
        executeBreak(breakStmt);
    } else if (auto* continueStmt = dynamic_cast<const ContinueStmt*>(stmt)) {
        executeContinue(continueStmt);
    } else if (auto* funcStmt = dynamic_cast<const FunctionStmt*>(stmt)) {
        executeFunctionDecl(funcStmt);
    } else if (auto* classStmt = dynamic_cast<const ClassStmt*>(stmt)) {
        executeClassDecl(classStmt);
    }
}

void Interpreter::executeExprStmt(const ExprStmt* stmt) {
    Value result = evaluateExpr(stmt->getExpression());
    // Expression statements evaluate but don't print the result
    (void)result;
}

void Interpreter::executeVarDecl(const VarDeclStmt* stmt) {
    Value value;
    if (stmt->getInitializer()) {
        value = evaluateExpr(stmt->getInitializer());
    }

    m_currentEnv->define(stmt->getName().getLexeme(), value);
}

void Interpreter::executeBlock(const BlockStmt* stmt, Environment* env) {
    Environment* previous = m_currentEnv;
    m_currentEnv = env;

    try {
        for (const auto& statement : stmt->getStatements()) {
            executeStmt(statement.get());
        }
    } catch (...) {
        m_currentEnv = previous;
        throw;
    }

    m_currentEnv = previous;
}

void Interpreter::executeIf(const IfStmt* stmt) {
    Value condition = evaluateExpr(stmt->getCondition());

    if (condition.asBool()) {
        executeStmt(stmt->getThenBranch());
    } else if (stmt->getElseBranch()) {
        executeStmt(stmt->getElseBranch());
    }
}

void Interpreter::executeWhile(const WhileStmt* stmt) {
    try {
        while (evaluateExpr(stmt->getCondition()).asBool()) {
            try {
                executeStmt(stmt->getBody());
            } catch (const ContinueException&) {
                continue;
            }
        }
    } catch (const BreakException&) {
        // Break out of the loop
    }
}

void Interpreter::executeFor(const ForStmt* stmt) {
    // Create new scope for the for loop
    Environment forEnv(m_currentEnv);
    Environment* previous = m_currentEnv;
    m_currentEnv = &forEnv;

    try {
        // Initialize
        if (stmt->getInitializer()) {
            executeStmt(stmt->getInitializer());
        }

        // Loop
        try {
            while (true) {
                // Check condition
                if (stmt->getCondition()) {
                    if (!evaluateExpr(stmt->getCondition()).asBool()) {
                        break;
                    }
                }

                // Execute body
                try {
                    executeStmt(stmt->getBody());
                } catch (const ContinueException&) {
                    // Continue to increment
                }

                // Increment
                if (stmt->getIncrement()) {
                    evaluateExpr(stmt->getIncrement());
                }
            }
        } catch (const BreakException&) {
            // Break out of the loop
        }
    } catch (...) {
        m_currentEnv = previous;
        throw;
    }

    m_currentEnv = previous;
}

void Interpreter::executeReturn(const ReturnStmt* stmt) {
    Value value;
    if (stmt->getValue()) {
        value = evaluateExpr(stmt->getValue());
    }
    throw ReturnException(value);
}

void Interpreter::executeBreak(const BreakStmt* stmt) {
    (void)stmt;
    throw BreakException();
}

void Interpreter::executeContinue(const ContinueStmt* stmt) {
    (void)stmt;
    throw ContinueException();
}

void Interpreter::executeFunctionDecl(const FunctionStmt* stmt) {
    // Create function object with current environment as closure
    auto function = std::make_shared<Function>(stmt, m_currentEnv);
    Value functionValue(function);
    m_currentEnv->define(stmt->getName().getLexeme(), functionValue);
}

void Interpreter::executeClassDecl(const ClassStmt* stmt) {
    // For now, store class name in environment
    // Full class support would require object value types
    m_currentEnv->define(stmt->getName().getLexeme(), Value());
}

// ===== Expression Evaluation =====

Value Interpreter::evaluateExpr(const Expr* expr) {
    if (auto* literal = dynamic_cast<const LiteralExpr*>(expr)) {
        return evaluateLiteral(literal);
    } else if (auto* variable = dynamic_cast<const VariableExpr*>(expr)) {
        return evaluateVariable(variable);
    } else if (auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
        return evaluateUnary(unary);
    } else if (auto* binary = dynamic_cast<const BinaryExpr*>(expr)) {
        return evaluateBinary(binary);
    } else if (auto* grouping = dynamic_cast<const GroupingExpr*>(expr)) {
        return evaluateGrouping(grouping);
    } else if (auto* assignment = dynamic_cast<const AssignmentExpr*>(expr)) {
        return evaluateAssignment(assignment);
    } else if (auto* logical = dynamic_cast<const LogicalExpr*>(expr)) {
        return evaluateLogical(logical);
    } else if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
        return evaluateCall(call);
    } else if (auto* array = dynamic_cast<const ArrayExpr*>(expr)) {
        return evaluateArray(array);
    } else if (auto* index = dynamic_cast<const IndexExpr*>(expr)) {
        return evaluateIndex(index);
    } else if (auto* member = dynamic_cast<const MemberExpr*>(expr)) {
        return evaluateMember(member);
    } else if (auto* map = dynamic_cast<const MapExpr*>(expr)) {
        return evaluateMap(map);
    }

    return Value();
}

Value Interpreter::evaluateLiteral(const LiteralExpr* expr) {
    const Token& token = expr->getToken();

    switch (token.getType()) {
        case TokenType::INTEGER:
            return Value(static_cast<i64>(std::stoll(token.getLexeme())));
        case TokenType::FLOAT:
            return Value(static_cast<f64>(std::stod(token.getLexeme())));
        case TokenType::STRING: {
            // Remove quotes from string literal
            String str = token.getLexeme();
            if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
                str = str.substr(1, str.size() - 2);
            }
            return Value(str);
        }
        case TokenType::TRUE:
            return Value(true);
        case TokenType::FALSE:
            return Value(false);
        case TokenType::NIL:
            return Value();
        default:
            return Value();
    }
}

Value Interpreter::evaluateVariable(const VariableExpr* expr) {
    try {
        return m_currentEnv->get(expr->getName().getLexeme());
    } catch (const std::exception& e) {
        runtimeError(e.what());
        return Value();
    }
}

Value Interpreter::evaluateUnary(const UnaryExpr* expr) {
    Value operand = evaluateExpr(expr->getOperand());

    switch (expr->getOperator().getType()) {
        case TokenType::NOT:
            return Value(!operand.asBool());
        case TokenType::MINUS:
            if (operand.isFloat()) {
                return Value(-operand.asFloat());
            } else if (operand.isInteger()) {
                return Value(-operand.asInteger());
            }
            runtimeError("Operand must be a number");
            return Value();
        case TokenType::PLUS:
            if (operand.isFloat()) {
                return Value(operand.asFloat());
            } else if (operand.isInteger()) {
                return Value(operand.asInteger());
            }
            runtimeError("Operand must be a number");
            return Value();
        default:
            return Value();
    }
}

Value Interpreter::evaluateBinary(const BinaryExpr* expr) {
    Value left = evaluateExpr(expr->getLeft());
    Value right = evaluateExpr(expr->getRight());

    switch (expr->getOperator().getType()) {
        // Arithmetic
        case TokenType::PLUS:
            if (left.isString() || right.isString()) {
                return Value(left.asString() + right.asString());
            } else if (left.isFloat() || right.isFloat()) {
                return Value(left.asFloat() + right.asFloat());
            } else if (left.isInteger() && right.isInteger()) {
                return Value(left.asInteger() + right.asInteger());
            }
            runtimeError("Operands must be numbers or strings");
            return Value();

        case TokenType::MINUS:
            if (left.isFloat() || right.isFloat()) {
                return Value(left.asFloat() - right.asFloat());
            } else if (left.isInteger() && right.isInteger()) {
                return Value(left.asInteger() - right.asInteger());
            }
            runtimeError("Operands must be numbers");
            return Value();

        case TokenType::STAR:
            if (left.isFloat() || right.isFloat()) {
                return Value(left.asFloat() * right.asFloat());
            } else if (left.isInteger() && right.isInteger()) {
                return Value(left.asInteger() * right.asInteger());
            }
            runtimeError("Operands must be numbers");
            return Value();

        case TokenType::SLASH:
            if (left.isFloat() || right.isFloat()) {
                f64 divisor = right.asFloat();
                if (divisor == 0.0) {
                    runtimeError("Division by zero");
                    return Value();
                }
                return Value(left.asFloat() / divisor);
            } else if (left.isInteger() && right.isInteger()) {
                i64 divisor = right.asInteger();
                if (divisor == 0) {
                    runtimeError("Division by zero");
                    return Value();
                }
                return Value(left.asInteger() / divisor);
            }
            runtimeError("Operands must be numbers");
            return Value();

        case TokenType::PERCENT:
            if (left.isInteger() && right.isInteger()) {
                i64 divisor = right.asInteger();
                if (divisor == 0) {
                    runtimeError("Modulo by zero");
                    return Value();
                }
                return Value(left.asInteger() % divisor);
            }
            runtimeError("Operands must be integers");
            return Value();

        case TokenType::POWER: {
            f64 base = left.asFloat();
            f64 exponent = right.asFloat();
            return Value(std::pow(base, exponent));
        }

        // Comparison
        case TokenType::GREATER:
            if (left.isFloat() || right.isFloat()) {
                return Value(left.asFloat() > right.asFloat());
            }
            return Value(left.asInteger() > right.asInteger());

        case TokenType::GREATER_EQUAL:
            if (left.isFloat() || right.isFloat()) {
                return Value(left.asFloat() >= right.asFloat());
            }
            return Value(left.asInteger() >= right.asInteger());

        case TokenType::LESS:
            if (left.isFloat() || right.isFloat()) {
                return Value(left.asFloat() < right.asFloat());
            }
            return Value(left.asInteger() < right.asInteger());

        case TokenType::LESS_EQUAL:
            if (left.isFloat() || right.isFloat()) {
                return Value(left.asFloat() <= right.asFloat());
            }
            return Value(left.asInteger() <= right.asInteger());

        // Equality
        case TokenType::EQUAL:
            if (left.getType() != right.getType()) {
                return Value(false);
            }
            if (left.isInteger()) {
                return Value(left.asInteger() == right.asInteger());
            } else if (left.isFloat()) {
                return Value(left.asFloat() == right.asFloat());
            } else if (left.isBool()) {
                return Value(left.asBool() == right.asBool());
            } else if (left.isString()) {
                return Value(left.asString() == right.asString());
            } else if (left.isNil()) {
                return Value(true);
            }
            return Value(false);

        case TokenType::NOT_EQUAL:
            if (left.getType() != right.getType()) {
                return Value(true);
            }
            if (left.isInteger()) {
                return Value(left.asInteger() != right.asInteger());
            } else if (left.isFloat()) {
                return Value(left.asFloat() != right.asFloat());
            } else if (left.isBool()) {
                return Value(left.asBool() != right.asBool());
            } else if (left.isString()) {
                return Value(left.asString() != right.asString());
            } else if (left.isNil()) {
                return Value(false);
            }
            return Value(true);

        default:
            return Value();
    }
}

Value Interpreter::evaluateGrouping(const GroupingExpr* expr) {
    return evaluateExpr(expr->getExpression());
}

Value Interpreter::evaluateAssignment(const AssignmentExpr* expr) {
    Value value = evaluateExpr(expr->getValue());

    try {
        m_currentEnv->set(expr->getName().getLexeme(), value);
    } catch (const std::exception& e) {
        runtimeError(e.what());
    }

    return value;
}

Value Interpreter::evaluateLogical(const LogicalExpr* expr) {
    Value left = evaluateExpr(expr->getLeft());

    if (expr->getOperator().getType() == TokenType::OR) {
        if (left.asBool()) {
            return left;
        }
    } else { // AND
        if (!left.asBool()) {
            return left;
        }
    }

    return evaluateExpr(expr->getRight());
}

Value Interpreter::evaluateCall(const CallExpr* expr) {
    // Evaluate the callee (should be a function)
    Value callee = evaluateExpr(expr->getCallee());

    if (!callee.isFunction()) {
        runtimeError("Can only call functions");
        return Value();
    }

    // Evaluate arguments
    std::vector<Value> arguments;
    for (const auto& arg : expr->getArguments()) {
        arguments.push_back(evaluateExpr(arg.get()));
    }

    FunctionType function = callee.asFunction();

    // Check arity
    if (arguments.size() != function->arity()) {
        runtimeError("Expected " + std::to_string(function->arity()) +
                    " arguments but got " + std::to_string(arguments.size()));
        return Value();
    }

    // Call native function
    if (function->isNative()) {
        return function->call(arguments, &m_globalEnv);
    }

    // Call user-defined function
    return callUserFunction(function, arguments);
}

Value Interpreter::evaluateArray(const ArrayExpr* expr) {
    std::vector<Value> elements;
    for (const auto& elemExpr : expr->getElements()) {
        elements.push_back(evaluateExpr(elemExpr.get()));
    }
    return Value::createArray(elements);
}

Value Interpreter::evaluateIndex(const IndexExpr* expr) {
    Value object = evaluateExpr(expr->getObject());
    Value index = evaluateExpr(expr->getIndex());

    if (object.isArray()) {
        if (!index.isInteger()) {
            runtimeError("Array index must be an integer");
            return Value();
        }
        i64 idx = index.asInteger();
        if (idx < 0) {
            runtimeError("Array index cannot be negative");
            return Value();
        }
        return object.arrayGet(static_cast<usize>(idx));
    } else if (object.isMap()) {
        if (!index.isString()) {
            runtimeError("Map key must be a string");
            return Value();
        }
        return object.mapGet(index.asString());
    }

    runtimeError("Can only index arrays and maps");
    return Value();
}

Value Interpreter::evaluateMember(const MemberExpr* expr) {
    // For now, member access is not fully implemented
    // This would require object value types
    (void)expr;
    runtimeError("Member access not yet implemented");
    return Value();
}

Value Interpreter::evaluateMap(const MapExpr* expr) {
    Value map = Value::createMap();
    for (const auto& pair : expr->getPairs()) {
        Value value = evaluateExpr(pair.second.get());
        map.mapSet(pair.first, value);
    }
    return map;
}

Value Interpreter::callUserFunction(const FunctionType& function, const std::vector<Value>& arguments) {
    // Get function declaration
    const FunctionStmt* declaration = function->getDeclaration();
    if (!declaration) {
        runtimeError("Invalid function");
        return Value();
    }

    // Create new environment for function scope (closure is the parent)
    Environment* closure = function->getClosure();
    Environment functionEnv(closure);

    // Bind parameters to arguments
    const auto& params = declaration->getParameters();
    for (usize i = 0; i < params.size(); ++i) {
        functionEnv.define(params[i].getLexeme(), arguments[i]);
    }

    // Save current environment and switch to function environment
    Environment* previousEnv = m_currentEnv;
    m_currentEnv = &functionEnv;

    // Execute function body and catch return
    Value returnValue;
    try {
        executeBlock(declaration->getBody(), &functionEnv);
    } catch (const ReturnException& e) {
        returnValue = e.getValue();
    }

    // Restore previous environment
    m_currentEnv = previousEnv;

    return returnValue;
}

Value Interpreter::callFunction(const String& name, const std::vector<Value>& arguments) {
    // Check if function exists
    if (!m_globalEnv.has(name)) {
        runtimeError("Undefined function: " + name);
        return Value();
    }

    try {
        Value functionValue = m_globalEnv.get(name);

        if (!functionValue.isFunction()) {
            runtimeError(name + " is not a function");
            return Value();
        }

        FunctionType function = functionValue.asFunction();

        // Check arity
        if (function->arity() != arguments.size()) {
            runtimeError("Function " + name + " expects " +
                        std::to_string(function->arity()) + " arguments, got " +
                        std::to_string(arguments.size()));
            return Value();
        }

        // Call native or user-defined function
        if (function->isNative()) {
            return function->call(arguments, &m_globalEnv);
        } else {
            return callUserFunction(function, arguments);
        }
    } catch (const std::exception& e) {
        runtimeError(String("Exception calling function: ") + e.what());
        return Value();
    }
}

} // namespace lang
} // namespace oracon
