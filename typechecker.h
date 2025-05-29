// typechecker.h
#pragma once
#include "ast.h"
#include "symboltable.h"
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

// Custom exception for type errors
class TypeError : public std::runtime_error {
public:
    explicit TypeError(const std::string& message) : std::runtime_error(message) {}
};

class TypeChecker {
private:
    SymbolTable symbolTable;
    
    // Current function return type for checking return statements
    TokenType currentFunctionReturnType;
    bool inFunctionBody;
    std::string currentFunctionName;
    
    // Type checking methods for expressions
    TokenType checkExpression(const Expression* expr);
    TokenType checkLiteral(const LiteralExpression* expr);
    TokenType checkIdentifier(const IdentifierExpression* expr);
    TokenType checkUnary(const UnaryExpression* expr);
    TokenType checkBinary(const BinaryExpression* expr);
    TokenType checkLogical(const LogicalExpression* expr);
    TokenType checkAssign(const AssignExpression* expr);
    TokenType checkCall(const CallExpression* expr);
    
    // Type checking methods for statements
    void checkStatement(const Statement* stmt);
    void checkExpressionStatement(const ExpressionStatement* stmt);
    void checkBlock(const BlockStatement* stmt);
    void checkVariableDeclaration(const VariableDeclaration* stmt);
    void checkFunctionDeclaration(const FunctionDeclaration* stmt);
    void checkIfStatement(const IfStatement* stmt);
    void checkWhileStatement(const WhileStatement* stmt);
    void checkForStatement(const ForStatement* stmt);
    void checkReturnStatement(const ReturnStatement* stmt);
    
    // Utility methods
    bool isNumericType(TokenType type) const;
    bool isBooleanType(TokenType type) const;
    bool isPointerType(TokenType type) const;
    bool isCompatibleType(TokenType left, TokenType right) const;
    TokenType getResultType(TokenType left, TokenType op, TokenType right) const;
    std::string tokenTypeToString(TokenType type) const;
    
public:
    TypeChecker();
    void check(const std::vector<std::unique_ptr<Statement>>& statements);
}; 