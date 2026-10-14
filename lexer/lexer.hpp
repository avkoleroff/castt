#pragma once
#include <string>

enum class TokenType {
    IDENTIFIER,
    INT,
    DOUBLE,
    CHAR,
    STRING,
    NEWLINE,
    SYMBOL,
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int col;
};

bool isLetter(char c) {
    if (isalpha(c)) return true;
    if ((unsigned char)c >= 0x80) return true;
    return false;
}

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isSymbol(char c) {
    std::string symbols = ".,:;!?-+=*/()[]{}<>_";
    return symbols.find(c) != std::string::npos;
}

class Lexer {
private:
    std::string source;
    size_t pos;
    int line;
    int col;

    char current() {
        if (pos >= source.length()) return '\0';
        return source[pos];
    }

    char peek(int offset = 1) {
        if (pos + offset >= source.length()) return '\0';
        return source[pos + offset];
    }

    void advance() {
        if (current() == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        pos++;
    }

    void skipWhitespace() {
        while (current() == ' ' || current() == '\t' || current() == '\r') {
            advance();
        }
    }

public:
    Lexer(const std::string& src) : source(src), pos(0), line(1), col(1) {}

    Token nextToken() {
        skipWhitespace();

        if (pos >= source.length()) {
            return {TokenType::END_OF_FILE, "EOF", line, col};
        }

        char c = current();
        int startLine = line;
        int startCol = col;

        if (c == '\n') {
            advance();
            return {TokenType::NEWLINE, "\\n", startLine, startCol};
        }

        if (isLetter(c)) {
            std::string val = "";
            while (isLetter(current()) || isDigit(current())) {
                val += current();
                advance();
            }
            return {TokenType::IDENTIFIER, val, startLine, startCol};
        }

        if (isDigit(c)) {
            std::string val = "";
            while (isDigit(current())) {
                val += current();
                advance();
            }
            if (current() == '.' && isDigit(peek())) {
                val += current();
                advance();
                while (isDigit(current())) {
                    val += current();
                    advance();
                }
                return {TokenType::DOUBLE, val, startLine, startCol};
            }
            return {TokenType::INT, val, startLine, startCol};
        }

        if (c == '"') {
            advance();
            std::string val = "";
            while (current() != '"' && current() != '\0') {
                if (current() == '\\' && (peek() == 'n' || peek() == 'r' || peek() == 't' || peek() == '\\' || peek() == '\'' || peek() == '"')) {
                    val += current(); advance();
                    val += current(); advance();
                } else {
                    val += current();
                    advance();
                }
            }
            if (current() == '"') advance();
            return {TokenType::STRING, val, startLine, startCol};
        }

        if (c == '\'') {
            advance();
            std::string val = "";
            while (current() != '\'' && current() != '\0' && current() != '\n') {
                if (current() == '\\' && (peek() == 'n' || peek() == 'r' || peek() == 't' || peek() == '\\' || peek() == '\'' || peek() == '"')) {
                    val += current(); advance();
                    val += current(); advance();
                } else {
                    val += current();
                    advance();
                }
            }
            if (current() == '\'') advance();
            return {TokenType::CHAR, val, startLine, startCol};
        }

        if (isSymbol(c)) {
            std::string val(1, c);
            advance();
            return {TokenType::SYMBOL, val, startLine, startCol};
        }
        advance();
        return {TokenType::UNKNOWN, std::string(1, c), startLine, startCol};
    }
};

std::string typeToString(TokenType type) {
    switch (type) {
        case TokenType::IDENTIFIER:  return "IDENTIFIER";
        case TokenType::INT:         return "INT";
        case TokenType::DOUBLE:      return "DOUBLE";
        case TokenType::CHAR:        return "CHAR";
        case TokenType::STRING:      return "std::string";
        case TokenType::NEWLINE:     return "NEWLINE";
        case TokenType::SYMBOL:      return "SYMBOL";
        case TokenType::END_OF_FILE: return "EOF";
        default:                     return "UNKNOWN";
    }
}