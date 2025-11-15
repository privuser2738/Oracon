#ifndef ORACON_LANG_LEXER_LEXER_H
#define ORACON_LANG_LEXER_LEXER_H

#include "oracon/lang/lexer/token.h"
#include "oracon/core/types.h"
#include <vector>
#include <unordered_map>

namespace oracon {
namespace lang {

class Lexer {
public:
    explicit Lexer(const String& source, const String& filename = "<stdin>");

    std::vector<Token> tokenize();
    Token nextToken();

    bool hasError() const { return m_hasError; }
    const std::vector<String>& getErrors() const { return m_errors; }

private:
    char peek(i32 offset = 0) const;
    char advance();
    bool match(char expected);
    bool isAtEnd() const;

    void skipWhitespace();
    void skipComment();

    Token makeToken(TokenType type);
    Token makeToken(TokenType type, const String& lexeme);
    Token errorToken(const String& message);

    Token scanString();
    Token scanNumber();
    Token scanIdentifier();

    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;

    TokenType identifierType();
    TokenType checkKeyword(usize start, usize length, const String& rest, TokenType type);

    void addError(const String& message);

private:
    String m_source;
    String m_filename;
    usize m_start;
    usize m_current;
    u32 m_line;
    u32 m_column;
    u32 m_startColumn;
    bool m_hasError;
    std::vector<String> m_errors;

    static const std::unordered_map<String, TokenType> s_keywords;
};

} // namespace lang
} // namespace oracon

#endif // ORACON_LANG_LEXER_LEXER_H
