#ifndef ORACON_LANG_LEXER_TOKEN_H
#define ORACON_LANG_LEXER_TOKEN_H

#include "oracon/core/types.h"
#include <string>
#include <ostream>

namespace oracon {
namespace lang {

using namespace core;

enum class TokenType {
    // Literals
    INTEGER,
    FLOAT,
    STRING,
    TRUE,
    FALSE,
    NIL,

    // Identifiers and keywords
    IDENTIFIER,
    LET,
    CONST,
    FUNC,
    CLASS,
    STATIC,
    IF,
    ELSE,
    WHILE,
    FOR,
    IN,
    RETURN,
    BREAK,
    CONTINUE,
    TRY,
    CATCH,
    FINALLY,
    THROW,
    IMPORT,
    EXPORT,
    FROM,
    AS,
    SELF,
    SUPER,
    MATCH,
    EXTENDS,
    INIT,

    // Operators
    PLUS,           // +
    MINUS,          // -
    STAR,           // *
    SLASH,          // /
    PERCENT,        // %
    POWER,          // **
    ASSIGN,         // =
    PLUS_ASSIGN,    // +=
    MINUS_ASSIGN,   // -=
    STAR_ASSIGN,    // *=
    SLASH_ASSIGN,   // /=
    PERCENT_ASSIGN, // %=

    // Comparison
    EQUAL,          // ==
    NOT_EQUAL,      // !=
    LESS,           // <
    LESS_EQUAL,     // <=
    GREATER,        // >
    GREATER_EQUAL,  // >=

    // Logical
    AND,            // and or &&
    OR,             // or or ||
    NOT,            // not or !

    // Delimiters
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]
    COMMA,          // ,
    DOT,            // .
    COLON,          // :
    SEMICOLON,      // ;
    ARROW,          // =>
    QUESTION,       // ?
    RANGE,          // ..
    SPREAD,         // ...

    // Special
    NEWLINE,
    EOF_TOKEN,
    INVALID
};

struct SourceLocation {
    String filename;
    u32 line;
    u32 column;

    SourceLocation(const String& file = "", u32 ln = 1, u32 col = 1)
        : filename(file), line(ln), column(col) {}
};

class Token {
public:
    Token(TokenType type, const String& lexeme, const SourceLocation& loc)
        : m_type(type)
        , m_lexeme(lexeme)
        , m_location(loc)
    {}

    TokenType getType() const { return m_type; }
    const String& getLexeme() const { return m_lexeme; }
    const SourceLocation& getLocation() const { return m_location; }

    bool is(TokenType type) const { return m_type == type; }
    bool isOneOf(TokenType t1, TokenType t2) const {
        return m_type == t1 || m_type == t2;
    }

    template<typename... Types>
    bool isOneOf(TokenType t1, TokenType t2, Types... rest) const {
        return m_type == t1 || isOneOf(t2, rest...);
    }

    String toString() const;

private:
    TokenType m_type;
    String m_lexeme;
    SourceLocation m_location;
};

const char* tokenTypeToString(TokenType type);

} // namespace lang
} // namespace oracon

#endif // ORACON_LANG_LEXER_TOKEN_H
