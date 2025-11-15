#include "oracon/lang/lexer/token.h"
#include <sstream>

namespace oracon {
namespace lang {

const char* tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::NIL: return "NIL";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::LET: return "LET";
        case TokenType::CONST: return "CONST";
        case TokenType::FUNC: return "FUNC";
        case TokenType::CLASS: return "CLASS";
        case TokenType::STATIC: return "STATIC";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::WHILE: return "WHILE";
        case TokenType::FOR: return "FOR";
        case TokenType::IN: return "IN";
        case TokenType::RETURN: return "RETURN";
        case TokenType::BREAK: return "BREAK";
        case TokenType::CONTINUE: return "CONTINUE";
        case TokenType::TRY: return "TRY";
        case TokenType::CATCH: return "CATCH";
        case TokenType::FINALLY: return "FINALLY";
        case TokenType::THROW: return "THROW";
        case TokenType::IMPORT: return "IMPORT";
        case TokenType::EXPORT: return "EXPORT";
        case TokenType::FROM: return "FROM";
        case TokenType::AS: return "AS";
        case TokenType::SELF: return "SELF";
        case TokenType::SUPER: return "SUPER";
        case TokenType::MATCH: return "MATCH";
        case TokenType::EXTENDS: return "EXTENDS";
        case TokenType::INIT: return "INIT";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::PERCENT: return "PERCENT";
        case TokenType::POWER: return "POWER";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::PLUS_ASSIGN: return "PLUS_ASSIGN";
        case TokenType::MINUS_ASSIGN: return "MINUS_ASSIGN";
        case TokenType::STAR_ASSIGN: return "STAR_ASSIGN";
        case TokenType::SLASH_ASSIGN: return "SLASH_ASSIGN";
        case TokenType::PERCENT_ASSIGN: return "PERCENT_ASSIGN";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::NOT_EQUAL: return "NOT_EQUAL";
        case TokenType::LESS: return "LESS";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::GREATER: return "GREATER";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::NOT: return "NOT";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::COMMA: return "COMMA";
        case TokenType::DOT: return "DOT";
        case TokenType::COLON: return "COLON";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::ARROW: return "ARROW";
        case TokenType::QUESTION: return "QUESTION";
        case TokenType::RANGE: return "RANGE";
        case TokenType::SPREAD: return "SPREAD";
        case TokenType::NEWLINE: return "NEWLINE";
        case TokenType::EOF_TOKEN: return "EOF";
        case TokenType::INVALID: return "INVALID";
        default: return "UNKNOWN";
    }
}

String Token::toString() const {
    std::ostringstream oss;
    oss << tokenTypeToString(m_type) << " '" << m_lexeme << "' at "
        << m_location.filename << ":" << m_location.line << ":" << m_location.column;
    return oss.str();
}

} // namespace lang
} // namespace oracon
