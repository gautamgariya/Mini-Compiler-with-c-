#pragma once
#include <string>

enum class TokenType {
    // Single-character tokens
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, MULTIPLY,
    AMPERSAND, PIPE,

    // One or two character tokens
    NOT, NOT_EQUAL,
    EQUAL, EQUAL_EQUAL,
    LESS, LESS_EQUAL, LEFT_SHIFT,
    GREATER, GREATER_EQUAL, RIGHT_SHIFT,
    AND, OR,
    INCREMENT, DECREMENT,
    ARROW,  // ->
    PLUS_EQUAL, MINUS_EQUAL, MULTIPLY_EQUAL, DIVIDE_EQUAL,  // +=, -=, *=, /=

    // Literals
    IDENTIFIER, STRING_LITERAL, CHAR_LITERAL,
    INTEGER_LITERAL, FLOAT_LITERAL, BOOL_LITERAL,

    // Keywords
    IF, ELSE, WHILE, FOR, RETURN,
    INT, FLOAT, CHAR, VOID, BOOL,
    USING, NAMESPACE, STD,
    COUT, CIN, ENDL,
    TRUE, FALSE,

    // Preprocessor
    HASH, INCLUDE,

    // Special
    POINTER,  // For type declarations
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;

    Token() : type(TokenType::END_OF_FILE), lexeme(""), line(0) {}  // Default constructor
    Token(TokenType type, const std::string& lexeme, int line)
        : type(type), lexeme(lexeme), line(line) {}
}; 