#pragma once

#include "token.hpp"
#include <string>
#include <vector>
#include <map>

class Lexer {
private:
    std::string source;
    size_t position;
    int line;
    int column;
    std::map<std::string, TokenType> keywords;
    std::vector<Token> tokens;

    void initializeKeywords();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    char advance();
    void skipWhitespace();
    void skipComment();
    bool isAtEnd() const;
    bool isDigit(char c) const { return c >= '0' && c <= '9'; }
    bool isAlpha(char c) const { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
    bool isAlphaNumeric(char c) const { return isAlpha(c) || isDigit(c); }
    
    Token identifier();
    Token number();
    Token string();
    Token character();
    Token preprocessorDirective();
    void addToken(TokenType type, const std::string& lexeme = "");

public:
    Lexer(const std::string& source);
    Token getNextToken();
    bool hasMoreTokens() const;
}; 