#include <bits/stdc++.h>
#include "lexer.hpp"

#define int long long
using namespace std;


void analysis() {
    std::string source((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());
    
    Lexer lexer(source);
    Token token;

    while ((token = lexer.nextToken()).type != TokenType::END_OF_FILE) {
        cout << "[" << typeToString(token.type) << "] \"" << token.value << "\" (" << token.line << ", " << token.col << ")\n";
    }
    
    cout << "[" << typeToString(token.type) << "] \"" << token.value << "\" (" << token.line << ", " << token.col << ")\n";
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