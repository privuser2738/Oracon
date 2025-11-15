#include "oracon/lang/lexer/lexer.h"
#include <cctype>
#include <sstream>

namespace oracon {
namespace lang {

const std::unordered_map<String, TokenType> Lexer::s_keywords = {
    {"let", TokenType::LET},
    {"const", TokenType::CONST},
    {"func", TokenType::FUNC},
    {"class", TokenType::CLASS},
    {"static", TokenType::STATIC},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"in", TokenType::IN},
    {"return", TokenType::RETURN},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"try", TokenType::TRY},
    {"catch", TokenType::CATCH},
    {"finally", TokenType::FINALLY},
    {"throw", TokenType::THROW},
    {"import", TokenType::IMPORT},
    {"export", TokenType::EXPORT},
    {"from", TokenType::FROM},
    {"as", TokenType::AS},
    {"self", TokenType::SELF},
    {"super", TokenType::SUPER},
    {"match", TokenType::MATCH},
    {"extends", TokenType::EXTENDS},
    {"init", TokenType::INIT},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"nil", TokenType::NIL},
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"not", TokenType::NOT},
};

Lexer::Lexer(const String& source, const String& filename)
    : m_source(source)
    , m_filename(filename)
    , m_start(0)
    , m_current(0)
    , m_line(1)
    , m_column(1)
    , m_startColumn(1)
    , m_hasError(false)
{}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        Token token = nextToken();
        tokens.push_back(token);

        if (token.getType() == TokenType::EOF_TOKEN) {
            break;
        }
    }

    return tokens;
}

Token Lexer::nextToken() {
    skipWhitespace();

    m_start = m_current;
    m_startColumn = m_column;

    if (isAtEnd()) {
        return makeToken(TokenType::EOF_TOKEN);
    }

    char c = advance();

    // Identifiers and keywords
    if (isAlpha(c)) {
        return scanIdentifier();
    }

    // Numbers
    if (isDigit(c)) {
        return scanNumber();
    }

    switch (c) {
        // String literals
        case '"':
        case '\'':
            return scanString();

        // Single character tokens
        case '(': return makeToken(TokenType::LPAREN);
        case ')': return makeToken(TokenType::RPAREN);
        case '{': return makeToken(TokenType::LBRACE);
        case '}': return makeToken(TokenType::RBRACE);
        case '[': return makeToken(TokenType::LBRACKET);
        case ']': return makeToken(TokenType::RBRACKET);
        case ',': return makeToken(TokenType::COMMA);
        case ':': return makeToken(TokenType::COLON);
        case ';': return makeToken(TokenType::SEMICOLON);
        case '?': return makeToken(TokenType::QUESTION);

        // Operators that can be multi-character
        case '+':
            return makeToken(match('=') ? TokenType::PLUS_ASSIGN : TokenType::PLUS);
        case '-':
            return makeToken(match('=') ? TokenType::MINUS_ASSIGN : TokenType::MINUS);
        case '%':
            return makeToken(match('=') ? TokenType::PERCENT_ASSIGN : TokenType::PERCENT);
        case '/':
            if (match('=')) {
                return makeToken(TokenType::SLASH_ASSIGN);
            } else if (match('/')) {
                // Single-line comment
                while (peek() != '\n' && !isAtEnd()) {
                    advance();
                }
                return nextToken(); // Skip the comment
            } else if (match('*')) {
                // Multi-line comment
                skipComment();
                return nextToken();
            }
            return makeToken(TokenType::SLASH);
        case '*':
            if (match('*')) {
                return makeToken(TokenType::POWER);
            } else if (match('=')) {
                return makeToken(TokenType::STAR_ASSIGN);
            }
            return makeToken(TokenType::STAR);
        case '!':
            if (match('=')) {
                return makeToken(TokenType::NOT_EQUAL);
            }
            return makeToken(TokenType::NOT);
        case '=':
            if (match('=')) {
                return makeToken(TokenType::EQUAL);
            } else if (match('>')) {
                return makeToken(TokenType::ARROW);
            }
            return makeToken(TokenType::ASSIGN);
        case '<':
            return makeToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        case '>':
            return makeToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        case '&':
            if (match('&')) {
                return makeToken(TokenType::AND);
            }
            break;
        case '|':
            if (match('|')) {
                return makeToken(TokenType::OR);
            }
            break;
        case '.':
            if (match('.')) {
                if (match('.')) {
                    return makeToken(TokenType::SPREAD);
                }
                return makeToken(TokenType::RANGE);
            }
            return makeToken(TokenType::DOT);
    }

    return errorToken("Unexpected character");
}

char Lexer::peek(i32 offset) const {
    usize pos = m_current + offset;
    if (pos >= m_source.length()) {
        return '\0';
    }
    return m_source[pos];
}

char Lexer::advance() {
    if (isAtEnd()) {
        return '\0';
    }

    char c = m_source[m_current++];

    if (c == '\n') {
        m_line++;
        m_column = 1;
    } else {
        m_column++;
    }

    return c;
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (m_source[m_current] != expected) return false;

    m_current++;
    m_column++;
    return true;
}

bool Lexer::isAtEnd() const {
    return m_current >= m_source.length();
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                advance();
                break;
            default:
                return;
        }
    }
}

void Lexer::skipComment() {
    // Multi-line comment /* ... */
    while (!isAtEnd()) {
        if (peek() == '*' && peek(1) == '/') {
            advance(); // *
            advance(); // /
            return;
        }
        advance();
    }

    addError("Unterminated comment");
}

Token Lexer::scanString() {
    char quote = m_source[m_start];
    String value;

    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\\') {
            advance(); // Skip backslash
            if (isAtEnd()) break;

            char escaped = advance();
            switch (escaped) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                case '\'': value += '\''; break;
                default: value += escaped; break;
            }
        } else {
            value += advance();
        }
    }

    if (isAtEnd()) {
        return errorToken("Unterminated string");
    }

    advance(); // Closing quote
    return makeToken(TokenType::STRING, value);
}

Token Lexer::scanNumber() {
    while (isDigit(peek())) {
        advance();
    }

    // Check for decimal point
    if (peek() == '.' && isDigit(peek(1))) {
        advance(); // Consume '.'
        while (isDigit(peek())) {
            advance();
        }
        return makeToken(TokenType::FLOAT);
    }

    // Check for scientific notation
    if (peek() == 'e' || peek() == 'E') {
        advance();
        if (peek() == '+' || peek() == '-') {
            advance();
        }
        while (isDigit(peek())) {
            advance();
        }
        return makeToken(TokenType::FLOAT);
    }

    return makeToken(TokenType::INTEGER);
}

Token Lexer::scanIdentifier() {
    while (isAlphaNumeric(peek())) {
        advance();
    }

    return makeToken(identifierType());
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

TokenType Lexer::identifierType() {
    String text = m_source.substr(m_start, m_current - m_start);
    auto it = s_keywords.find(text);
    if (it != s_keywords.end()) {
        return it->second;
    }
    return TokenType::IDENTIFIER;
}

Token Lexer::makeToken(TokenType type) {
    String lexeme = m_source.substr(m_start, m_current - m_start);
    SourceLocation loc(m_filename, m_line, m_startColumn);
    return Token(type, lexeme, loc);
}

Token Lexer::makeToken(TokenType type, const String& lexeme) {
    SourceLocation loc(m_filename, m_line, m_startColumn);
    return Token(type, lexeme, loc);
}

Token Lexer::errorToken(const String& message) {
    addError(message);
    SourceLocation loc(m_filename, m_line, m_startColumn);
    return Token(TokenType::INVALID, message, loc);
}

void Lexer::addError(const String& message) {
    std::ostringstream oss;
    oss << m_filename << ":" << m_line << ":" << m_startColumn << ": " << message;
    m_errors.push_back(oss.str());
    m_hasError = true;
}

} // namespace lang
} // namespace oracon
