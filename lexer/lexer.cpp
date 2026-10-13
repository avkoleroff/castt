#include <bits/stdc++.h>
using namespace std;

enum class TokenType {
    IDENTIFIER,
    INT,
    DOUBLE,
    CHAR,
    STRING_LIT,
    NEWLINE,
    SYMBOL,
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType type;
    string value;
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
    string symbols = ".,:;!?-+=*/()[]{}<>_";
    return symbols.find(c) != string::npos;
}

class Lexer {
private:
    string source;
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
    Lexer(const string& src) : source(src), pos(0), line(1), col(1) {}

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
            string val = "";
            while (isLetter(current()) || isDigit(current())) {
                val += current();
                advance();
            }
            return {TokenType::IDENTIFIER, val, startLine, startCol};
        }

        if (isDigit(c)) {
            string val = "";
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
            string val = "";
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
            return {TokenType::STRING_LIT, val, startLine, startCol};
        }

        if (c == '\'') {
            advance();
            string val = "";
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
            string val(1, c);
            advance();
            return {TokenType::SYMBOL, val, startLine, startCol};
        }
        advance();
        return {TokenType::UNKNOWN, string(1, c), startLine, startCol};
    }
};

string typeToString(TokenType type) {
    switch (type) {
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::INT:    return "INT";
        case TokenType::DOUBLE: return "DOUBLE";
        case TokenType::CHAR:   return "CHAR";
        case TokenType::STRING_LIT: return "STRING";
        case TokenType::NEWLINE:    return "NEWLINE";
        case TokenType::SYMBOL:     return "SYMBOL";
        case TokenType::END_OF_FILE:return "EOF";
        default:                    return "UNKNOWN";
    }
}

void analysis() {
    string source((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());
    
    Lexer lexer(source);
    Token token;

    while ((token = lexer.nextToken()).type != TokenType::END_OF_FILE) {
        cout << "[" << typeToString(token.type) << "] \"" 
             << token.value << "\" (" << token.line << ", " << token.col << ")\n";
    }
    
    cout << "[" << typeToString(token.type) << "] \"" 
         << token.value << "\" (" << token.line << ", " << token.col << ")\n";
}

signed main() {
    cin.tie(0);
    cout.tie(0);
    ios::sync_with_stdio(0);
    
    freopen("example.txt", "r", stdin);
    freopen("lex_analysis.txt", "w", stdout);
    
    analysis();
    
    return 0;
}