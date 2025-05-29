#pragma once

#include "lexer.h"
#include "ast.h"
#include <memory>
#include <vector>

// Forward declarations
class Type;

class Parser {
private:
    Lexer& lexer;
    Token currentToken;

    void advance();
    bool match(TokenType type);
    bool check(TokenType type) const;
    void consume(TokenType type, const std::string& message);
    Token peek() const;
    void synchronize();
    
    // Parsing methods for expressions
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseAssignment();
    std::unique_ptr<Expression> parseLogicalOr();
    std::unique_ptr<Expression> parseLogicalAnd();
    std::unique_ptr<Expression> parseEquality();
    std::unique_ptr<Expression> parseComparison();
    std::unique_ptr<Expression> parseTerm();
    std::unique_ptr<Expression> parseFactor();
    std::unique_ptr<Expression> parsePrimary();
    
    // Parsing methods for statements
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Statement> parseBlock();
    std::unique_ptr<Statement> parseIfStatement();
    std::unique_ptr<Statement> parseFunctionOrVariableDeclaration();
    std::unique_ptr<Statement> parsePreprocessorDirective();
    std::unique_ptr<Statement> parseUsingDirective();

public:
    Parser(Lexer& lexer);
    std::vector<std::unique_ptr<Statement>> parse();
}; 