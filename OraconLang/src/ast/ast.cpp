#include "oracon/lang/ast/ast.h"
#include <sstream>

namespace oracon {
namespace lang {

// ===== Expression toString() implementations =====

String UnaryExpr::toString() const {
    std::ostringstream oss;
    oss << "(" << m_operator.getLexeme() << m_operand->toString() << ")";
    return oss.str();
}

String BinaryExpr::toString() const {
    std::ostringstream oss;
    oss << "(" << m_left->toString() << " " << m_operator.getLexeme() << " " << m_right->toString() << ")";
    return oss.str();
}

String GroupingExpr::toString() const {
    return "(" + m_expr->toString() + ")";
}

String AssignmentExpr::toString() const {
    return m_name.getLexeme() + " = " + m_value->toString();
}

String LogicalExpr::toString() const {
    std::ostringstream oss;
    oss << "(" << m_left->toString() << " " << m_operator.getLexeme() << " " << m_right->toString() << ")";
    return oss.str();
}

String CallExpr::toString() const {
    std::ostringstream oss;
    oss << m_callee->toString() << "(";
    for (size_t i = 0; i < m_arguments.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << m_arguments[i]->toString();
    }
    oss << ")";
    return oss.str();
}

String ArrayExpr::toString() const {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < m_elements.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << m_elements[i]->toString();
    }
    oss << "]";
    return oss.str();
}

String IndexExpr::toString() const {
    return m_object->toString() + "[" + m_index->toString() + "]";
}

String MemberExpr::toString() const {
    return m_object->toString() + "." + m_member.getLexeme();
}

String MapExpr::toString() const {
    std::ostringstream oss;
    oss << "{";
    for (size_t i = 0; i < m_pairs.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << m_pairs[i].first << ": " << m_pairs[i].second->toString();
    }
    oss << "}";
    return oss.str();
}

// ===== Statement toString() implementations =====

String ExprStmt::toString() const {
    return m_expr->toString() + ";";
}

String VarDeclStmt::toString() const {
    std::ostringstream oss;
    oss << (m_isConst ? "const " : "let ") << m_name.getLexeme();
    if (m_initializer) {
        oss << " = " << m_initializer->toString();
    }
    return oss.str();
}

String FunctionStmt::toString() const {
    std::ostringstream oss;
    oss << "func " << m_name.getLexeme() << "(";
    for (size_t i = 0; i < m_parameters.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << m_parameters[i].getLexeme();
    }
    oss << ") { ... }";
    return oss.str();
}

String ClassStmt::toString() const {
    std::ostringstream oss;
    oss << "class " << m_name.getLexeme() << " { " << m_methods.size() << " methods }";
    return oss.str();
}

} // namespace lang
} // namespace oracon
