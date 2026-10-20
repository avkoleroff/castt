#pragma once
#include <string>
#include <unordered_map>
#include <cctype>
#include <vector>

enum class TokenType {
    IDENTIFIER,
    KEYWORD,
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
    if (std::isalpha(static_cast<unsigned char>(c))) return true;
    if ((unsigned char)c >= 0x80) return true;
    return false;
}

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isSymbol(char c) {
    std::string symbols = ".,:;!?-+=*/()[]{}<>_|&^~%";
    return symbols.find(c) != std::string::npos;
}

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

std::string normalizeSpaces(const std::string& s) {
    std::string res;
    bool inSpace = false;
    for (char c : s) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!inSpace) {
                res += ' ';
                inSpace = true;
            }
        } else {
            res += c;
            inSpace = false;
        }
    }
    return res;
}

struct TrieNode {
    std::unordered_map<unsigned char, TrieNode*> children;
    bool isEnd = false;
    
    ~TrieNode() {
        for (auto& pair : children) {
            delete pair.second;
        }
    }
};

class KeywordTrie {
private:
    TrieNode* root;

public:
    KeywordTrie() { root = new TrieNode(); }
    ~KeywordTrie() { delete root; }

    void insert(const std::string& keyword) {
        std::string cleanKw = trim(keyword);
        if (cleanKw.empty()) return;
        
        TrieNode* current = root;
        for (unsigned char c : cleanKw) {
            if (current->children.find(c) == current->children.end()) {
                current->children[c] = new TrieNode();
            }
            current = current->children[c];
        }
        current->isEnd = true;
    }

    size_t match(const std::string& source, size_t startPos) {
        TrieNode* current = root;
        size_t pos = startPos;
        
        while (pos < source.length()) {
            unsigned char c = source[pos];
            
            if (std::isspace(c)) {
                if (current->children.find(' ') != current->children.end()) {
                    current = current->children[' '];
                    while (pos < source.length() && std::isspace(source[pos])) {
                        pos++;
                    }
                    continue;
                } else {
                    break;
                }
            }
            
            if (current->children.find(c) != current->children.end()) {
                current = current->children[c];
                pos++;
            } else {
                break;
            }
        }
        
        if (current->isEnd) {
            return pos - startPos;
        }
        return 0;
    }
};

class Lexer {
private:
    std::string source;
    size_t pos;
    int line;
    int col;
    KeywordTrie trie;

    void initKeywords() {
        std::vector<std::string> keywords = {
            "Да будет", 
            "со значением", 
            "Да примет", 
            "значение",
            "Взываю к Слуху с даром", 
            "Взываю к Голосу с даром",
            "Если истинно предзнаменование", 
            "да свершится следующее",
            "Если же нет", 
            "да свершится иное", 
            "Да свершится предначертанное",
            "О великие силы повторения", 
            "взываю к вам",
            "Покуда истинно предзнаменование", 
            "вершите", 
            "Да замкнётся круг",
            "О хранители пути", 
            "проведите", 
            "от", 
            "до", 
            "с шагом", 
            "Да завершится шествие",
            "Наречём новый облик", 
            "Да завершится облик",
            "Наречём перечень",
            "Да завершится перечень",
            "воплощение",
            "Запишем заклинание", 
            "с дарами", 
            "возвращающее", 
            "Да запечатается заклинание",
            "Исполнение заклинания",
            "Да исполнится", 
            "Да станет ответом", 
            "Да завершится призыв",
            "цепь", 
            "из",
            "целое",
            "дробное",
            "логическое",
            "символ",
            "строка",
            "ничто",
            "истина",
            "ложь",
            "взываю к Преображению с даром",
            "в"
        };

        for (const auto& kw : keywords) {
            trie.insert(kw);
        }
    }

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
    Lexer(const std::string& src) : source(src), pos(0), line(1), col(1) {
        initKeywords();
    }

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
            size_t savedPos = pos;
            int savedLine = line;
            int savedCol = col;
            
            size_t matchLen = trie.match(source, pos);
            
            if (matchLen > 0) {
                size_t endPos = pos + matchLen;
                bool isWordBoundary = true;
                if (endPos < source.length()) {
                    char nextC = source[endPos];
                    if (isLetter(nextC) || isDigit(nextC)) {
                        isWordBoundary = false;
                    }
                }
                
                if (isWordBoundary) {
                    std::string val = source.substr(pos, matchLen);
                    pos = endPos;
                    for (size_t i = 0; i < matchLen; ++i) {
                        if (source[savedPos + i] == '\n') { line++; col = 1; }
                        else { col++; }
                    }
                    return {TokenType::KEYWORD, normalizeSpaces(val), startLine, startCol};
                }
            }
            
            pos = savedPos;
            line = savedLine;
            col = savedCol;
            
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
            while (current() != '"' && current() != '\0' && current() != '\n') {
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
            
            if (c == '|' && current() == '|') { val += current(); advance(); }
            else if (c == '&' && current() == '&') { val += current(); advance(); }
            else if (c == '=' && current() == '=') { val += current(); advance(); }
            else if (c == '!' && current() == '=') { val += current(); advance(); }
            else if (c == '>' && current() == '=') { val += current(); advance(); }
            else if (c == '<' && current() == '=') { val += current(); advance(); }
            else if (c == '>' && current() == '>') { val += current(); advance(); }
            else if (c == '<' && current() == '<') { val += current(); advance(); }
            
            return {TokenType::SYMBOL, val, startLine, startCol};
        }

        advance();
        return {TokenType::UNKNOWN, std::string(1, c), startLine, startCol};
    }
};

std::string typeToString(TokenType type) {
    switch (type) {
        case TokenType::IDENTIFIER:  return "IDENTIFIER";
        case TokenType::KEYWORD:     return "KEYWORD";
        case TokenType::INT:         return "INT";
        case TokenType::DOUBLE:      return "DOUBLE";
        case TokenType::CHAR:        return "CHAR";
        case TokenType::STRING:      return "STRING";
        case TokenType::NEWLINE:     return "NEWLINE";
        case TokenType::SYMBOL:      return "SYMBOL";
        case TokenType::END_OF_FILE: return "EOF";
        default:                     return "UNKNOWN";
    }
}