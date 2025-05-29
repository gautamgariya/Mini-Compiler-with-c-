#include "../include/lexer.h"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& source) : source(source), position(0), line(1), column(1) {
    initializeKeywords();
}

void Lexer::initializeKeywords() {
    keywords["int"] = TokenType::INT;
    keywords["float"] = TokenType::FLOAT;
    keywords["char"] = TokenType::CHAR;
    keywords["void"] = TokenType::VOID;
    keywords["bool"] = TokenType::BOOL;
    keywords["string"] = TokenType::STRING_LITERAL;
    keywords["if"] = TokenType::IF;
    keywords["else"] = TokenType::ELSE;
    keywords["while"] = TokenType::WHILE;
    keywords["for"] = TokenType::FOR;
    keywords["return"] = TokenType::RETURN;
    keywords["true"] = TokenType::TRUE;
    keywords["false"] = TokenType::FALSE;
    keywords["cout"] = TokenType::COUT;
    keywords["cin"] = TokenType::CIN;
    keywords["endl"] = TokenType::ENDL;
    keywords["using"] = TokenType::USING;
    keywords["namespace"] = TokenType::NAMESPACE;
    keywords["std"] = TokenType::STD;
    keywords["include"] = TokenType::INCLUDE;
}

char Lexer::peek() const {
    if (position >= source.length()) return '\0';
    return source[position];
}

bool Lexer::match(char expected) {
    if (peek() == expected) {
        advance();
        return true;
    }
    return false;
}

char Lexer::advance() {
    char current = peek();
    position++;
    if (current == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return current;
}

void Lexer::skipWhitespace() {
    while (isspace(peek())) {
        advance();
    }
}

void Lexer::skipComment() {
    if (peek() == '/' && position + 1 < source.length()) {
        if (source[position + 1] == '/') {
            // Single-line comment
            while (peek() != '\n' && peek() != '\0') {
                advance();
            }
        } else if (source[position + 1] == '*') {
            // Multi-line comment
            advance(); // Skip /
            advance(); // Skip *
            while (position + 1 < source.length()) {
                if (peek() == '*' && source[position + 1] == '/') {
                    advance(); // Skip *
                    advance(); // Skip /
                    break;
                }
                advance();
            }
        }
    }
}

Token Lexer::identifier() {
    std::string lexeme;
    
    while (isalnum(peek()) || peek() == '_') {
        lexeme += advance();
    }
    
    auto it = keywords.find(lexeme);
    if (it != keywords.end()) {
        return Token(it->second, lexeme, line);
    }
    
    return Token(TokenType::IDENTIFIER, lexeme, line);
}

Token Lexer::number() {
    std::string lexeme;
    bool isFloat = false;
    
    while (isdigit(peek()) || peek() == '.') {
        if (peek() == '.') {
            if (isFloat) break;
            isFloat = true;
        }
        lexeme += advance();
    }
    
    return Token(isFloat ? TokenType::FLOAT_LITERAL : TokenType::INTEGER_LITERAL,
                lexeme, line);
}

Token Lexer::string() {
    std::string lexeme;
    
    while (peek() != '"' && peek() != '\0') {
        if (peek() == '\\') {
            advance();
            switch (peek()) {
                case 'n': lexeme += '\n'; break;
                case 't': lexeme += '\t'; break;
                case 'r': lexeme += '\r'; break;
                case '\\': lexeme += '\\'; break;
                case '"': lexeme += '"'; break;
                default: lexeme += peek();
            }
            advance();
        } else {
            lexeme += advance();
        }
    }
    
    if (peek() == '"') {
        advance(); // Consume closing quote
        return Token(TokenType::STRING_LITERAL, lexeme, line);
    }
    
    throw std::runtime_error("Unterminated string");
}

Token Lexer::character() {
    std::string value;
    
    if (peek() == '\\') {
        advance(); // Skip backslash
        switch (peek()) {
            case 'n': value = "\n"; break;
            case 't': value = "\t"; break;
            case 'r': value = "\r"; break;
            case '\\': value = "\\"; break;
            case '\'': value = "'"; break;
            default: value = std::string(1, peek());
        }
        advance();
    } else {
        value = std::string(1, advance());
    }
    
    if (peek() == '\'') {
        advance(); // Skip closing quote
        return Token(TokenType::CHAR_LITERAL, value, line);
    }
    
    throw std::runtime_error("Invalid character literal");
}

Token Lexer::preprocessorDirective() {
    std::string lexeme;
    
    // Skip the #
    advance();
    skipWhitespace();
    
    // Read the directive name
    while (isalpha(peek())) {
        lexeme += advance();
    }
    
    if (lexeme == "include") {
        skipWhitespace();
        // Handle both <header> and "header" formats
        if (peek() == '<' || peek() == '"') {
            char terminator = (peek() == '<') ? '>' : '"';
            advance(); // Skip opening < or "
            std::string header;
            while (peek() != terminator && peek() != '\0') {
                header += advance();
            }
            if (peek() == terminator) {
                advance(); // Skip closing > or "
            }
            return Token(TokenType::INCLUDE, header, line);
        }
    }
    
    return Token(TokenType::HASH, lexeme, line);
}

Token Lexer::getNextToken() {
    skipWhitespace();
    
    if (position >= source.length()) {
        return Token(TokenType::END_OF_FILE, "", line);
    }
    
    char c = peek();

    // Handle comments
    if (c == '/' && (position + 1 < source.length()) && 
        (source[position + 1] == '/' || source[position + 1] == '*')) {
        skipComment();
        return getNextToken();
    }
    
    // Handle preprocessor directives
    if (c == '#') {
        return preprocessorDirective();
    }
    
    if (isalpha(c) || c == '_') {
        return identifier();
    }
    
    if (isdigit(c)) {
        return number();
    }
    
    advance(); // Consume the character
    
    switch (c) {
        case '+':
            if (match('+')) return Token(TokenType::INCREMENT, "++", line);
            if (match('=')) return Token(TokenType::PLUS_EQUAL, "+=", line);
            return Token(TokenType::PLUS, "+", line);
        case '-':
            if (match('-')) return Token(TokenType::DECREMENT, "--", line);
            if (match('=')) return Token(TokenType::MINUS_EQUAL, "-=", line);
            if (match('>')) return Token(TokenType::ARROW, "->", line);
            return Token(TokenType::MINUS, "-", line);
        case '*':
            if (match('=')) return Token(TokenType::MULTIPLY_EQUAL, "*=", line);
            return Token(TokenType::MULTIPLY, "*", line);
        case '/':
            if (match('=')) return Token(TokenType::DIVIDE_EQUAL, "/=", line);
            return Token(TokenType::SLASH, "/", line);
        case '(':
            return Token(TokenType::LPAREN, "(", line);
        case ')':
            return Token(TokenType::RPAREN, ")", line);
        case '{':
            return Token(TokenType::LBRACE, "{", line);
        case '}':
            return Token(TokenType::RBRACE, "}", line);
        case '[':
            return Token(TokenType::LBRACKET, "[", line);
        case ']':
            return Token(TokenType::RBRACKET, "]", line);
        case ';':
            return Token(TokenType::SEMICOLON, ";", line);
        case ',':
            return Token(TokenType::COMMA, ",", line);
        case '.':
            return Token(TokenType::DOT, ".", line);
        case '&':
            if (match('&')) return Token(TokenType::AND, "&&", line);
            return Token(TokenType::AMPERSAND, "&", line);
        case '|':
            if (match('|')) return Token(TokenType::OR, "||", line);
            return Token(TokenType::PIPE, "|", line);
        case '<':
            if (match('=')) return Token(TokenType::LESS_EQUAL, "<=", line);
            if (match('<')) return Token(TokenType::LEFT_SHIFT, "<<", line);
            return Token(TokenType::LESS, "<", line);
        case '>':
            if (match('=')) return Token(TokenType::GREATER_EQUAL, ">=", line);
            if (match('>')) return Token(TokenType::RIGHT_SHIFT, ">>", line);
            return Token(TokenType::GREATER, ">", line);
        case '=':
            if (match('=')) return Token(TokenType::EQUAL_EQUAL, "==", line);
            return Token(TokenType::EQUAL, "=", line);
        case '!':
            if (match('=')) return Token(TokenType::NOT_EQUAL, "!=", line);
            return Token(TokenType::NOT, "!", line);
        case '"':
            return string();
        case '\'':
            return character();
    }
    
    throw std::runtime_error("Unexpected character: " + std::string(1, c));
}

bool Lexer::hasMoreTokens() const {
    return position < source.length();
}

void Lexer::addToken(TokenType type, const std::string& lexeme) {
    std::string tokenLexeme = lexeme;
    if (lexeme.empty()) {
        char c = source[position - 1];  // Use the last character we processed
        tokenLexeme = std::string(1, c);
    }
    tokens.push_back(Token(type, tokenLexeme, line));
}

char Lexer::peekNext() const {
    if (position + 1 >= source.length()) return '\0';
    return source[position + 1];
}

bool Lexer::isAtEnd() const {
    return position >= source.length();
} 