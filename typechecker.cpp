// typechecker.cpp
#include "../include/typechecker.h"
#include <iostream>
#include <sstream>

TypeChecker::TypeChecker() : inFunctionBody(false), currentFunctionReturnType(TokenType::VOID) {}

void TypeChecker::check(const std::vector<std::unique_ptr<Statement>>& statements) {
    std::vector<std::string> errors;
    
    // First pass: register all function declarations for forward references
    for (const auto& stmt : statements) {
        if (auto* funcDecl = dynamic_cast<const FunctionDeclaration*>(stmt.get())) {
            std::vector<std::pair<std::string, TokenType>> params;
            for (const auto& param : funcDecl->getParameters()) {
                params.push_back(param);
            }
            
            Symbol funcSymbol(funcDecl->getName(), funcDecl->getReturnType(), std::move(params));
            if (!symbolTable.define(funcSymbol)) {
                std::stringstream ss;
                ss << "Function '" << funcDecl->getName() << "' already defined";
                errors.push_back(ss.str());
            }
        }
    }
    
    // Second pass: check all statements
    for (const auto& stmt : statements) {
        try {
            checkStatement(stmt.get());
        } catch (const TypeError& e) {
            errors.push_back(e.what());
        }
    }
    
    // Report all errors
    if (!errors.empty()) {
        std::stringstream ss;
        ss << "Found " << errors.size() << " semantic errors:" << std::endl;
        for (const auto& error : errors) {
            ss << "- " << error << std::endl;
        }
        throw TypeError(ss.str());
    }
}

TokenType TypeChecker::checkExpression(const Expression* expr) {
    if (auto* literalExpr = dynamic_cast<const LiteralExpression*>(expr)) {
        return checkLiteral(literalExpr);
    } else if (auto* identifierExpr = dynamic_cast<const IdentifierExpression*>(expr)) {
        return checkIdentifier(identifierExpr);
    } else if (auto* unaryExpr = dynamic_cast<const UnaryExpression*>(expr)) {
        return checkUnary(unaryExpr);
    } else if (auto* binaryExpr = dynamic_cast<const BinaryExpression*>(expr)) {
        return checkBinary(binaryExpr);
    } else if (auto* logicalExpr = dynamic_cast<const LogicalExpression*>(expr)) {
        return checkLogical(logicalExpr);
    } else if (auto* assignExpr = dynamic_cast<const AssignExpression*>(expr)) {
        return checkAssign(assignExpr);
    } else if (auto* callExpr = dynamic_cast<const CallExpression*>(expr)) {
        return checkCall(callExpr);
    }
    
    throw TypeError("Unknown expression type");
}

TokenType TypeChecker::checkLiteral(const LiteralExpression* expr) {
    return expr->getLiteralType();
}

TokenType TypeChecker::checkIdentifier(const IdentifierExpression* expr) {
    Symbol* symbol = symbolTable.resolve(expr->getName());
    if (!symbol) {
        std::stringstream ss;
        ss << "Undefined variable '" << expr->getName() << "'";
        throw TypeError(ss.str());
    }
    
    if (symbol->kind == Symbol::SymbolKind::FUNCTION) {
        std::stringstream ss;
        ss << "'" << expr->getName() << "' is a function and cannot be used as a variable";
        throw TypeError(ss.str());
    }
    
    return symbol->isPointer ? TokenType::POINTER : symbol->type;
}

TokenType TypeChecker::checkUnary(const UnaryExpression* expr) {
    TokenType rightType = checkExpression(expr->getOperand());
    TokenType op = expr->getOperator();
    
    switch (op) {
        case TokenType::MINUS:
        case TokenType::PLUS:
            if (!isNumericType(rightType)) {
                throw TypeError("Unary '+' and '-' operators require numeric operands");
            }
            return rightType;
            
        case TokenType::NOT:
            return TokenType::BOOL;
            
        case TokenType::INCREMENT:
        case TokenType::DECREMENT:
            if (!isNumericType(rightType)) {
                throw TypeError("Increment and decrement operators require numeric operands");
            }
            return rightType;
            
        case TokenType::MULTIPLY: // Dereference operator
            if (rightType != TokenType::POINTER) {
                throw TypeError("Cannot dereference non-pointer type");
            }
            // Return the base type, not the pointer type
            return TokenType::INT; // Simplified - should return the actual pointed type
            
        case TokenType::AMPERSAND: // Address-of operator
            // Return pointer to the type
            return TokenType::POINTER;
            
        default:
            std::stringstream ss;
            ss << "Unsupported unary operator: " << static_cast<int>(op);
            throw TypeError(ss.str());
    }
}

TokenType TypeChecker::checkBinary(const BinaryExpression* expr) {
    TokenType leftType = checkExpression(expr->getLeft());
    TokenType rightType = checkExpression(expr->getRight());
    TokenType op = expr->getOperator();
    
    // Special case for stream operators
    if (op == TokenType::LEFT_SHIFT || op == TokenType::RIGHT_SHIFT) {
        // Simplified check - in a real compiler, we'd verify these are actually stream objects
        return leftType;
    }
    
    // Check arithmetic operators
    if (op == TokenType::PLUS || op == TokenType::MINUS || 
        op == TokenType::MULTIPLY || op == TokenType::SLASH) {
        
        // String concatenation with +
        if (op == TokenType::PLUS && 
            (leftType == TokenType::STRING_LITERAL || rightType == TokenType::STRING_LITERAL)) {
            return TokenType::STRING_LITERAL;
        }
        
        // Pointer arithmetic
        if (op == TokenType::PLUS || op == TokenType::MINUS) {
            if (leftType == TokenType::POINTER && isNumericType(rightType)) {
                return TokenType::POINTER;
            }
            if (rightType == TokenType::POINTER && op == TokenType::PLUS && isNumericType(leftType)) {
                return TokenType::POINTER;
            }
        }
        
        // Regular arithmetic
        if (!isNumericType(leftType) || !isNumericType(rightType)) {
            std::stringstream ss;
            ss << "Binary operator '" << tokenTypeToString(op) 
               << "' requires numeric operands, got " 
               << tokenTypeToString(leftType) << " and " 
               << tokenTypeToString(rightType);
            throw TypeError(ss.str());
        }
        
        // Determine result type (float if either operand is float)
        if (leftType == TokenType::FLOAT_LITERAL || rightType == TokenType::FLOAT_LITERAL) {
            return TokenType::FLOAT_LITERAL;
        }
        return TokenType::INTEGER_LITERAL;
    }
    
    // Comparison operators
    if (op == TokenType::EQUAL_EQUAL || op == TokenType::NOT_EQUAL ||
        op == TokenType::LESS || op == TokenType::LESS_EQUAL ||
        op == TokenType::GREATER || op == TokenType::GREATER_EQUAL) {
        
        if (!isCompatibleType(leftType, rightType)) {
            std::stringstream ss;
            ss << "Cannot compare incompatible types: " 
               << tokenTypeToString(leftType) << " and " 
               << tokenTypeToString(rightType);
            throw TypeError(ss.str());
        }
        
        return TokenType::BOOL;
    }
    
    std::stringstream ss;
    ss << "Unsupported binary operator: " << static_cast<int>(op);
    throw TypeError(ss.str());
}

TokenType TypeChecker::checkLogical(const LogicalExpression* expr) {
    TokenType leftType = checkExpression(expr->getLeft());
    TokenType rightType = checkExpression(expr->getRight());
    
    if (!isBooleanType(leftType)) {
        std::stringstream ss;
        ss << "Left operand of logical operator must be boolean, got " 
           << tokenTypeToString(leftType);
        throw TypeError(ss.str());
    }
    
    if (!isBooleanType(rightType)) {
        std::stringstream ss;
        ss << "Right operand of logical operator must be boolean, got " 
           << tokenTypeToString(rightType);
        throw TypeError(ss.str());
    }
    
    return TokenType::BOOL;
}

TokenType TypeChecker::checkAssign(const AssignExpression* expr) {
    Symbol* symbol = symbolTable.resolve(expr->getName());
    if (!symbol) {
        std::stringstream ss;
        ss << "Cannot assign to undeclared variable '" << expr->getName() << "'";
        throw TypeError(ss.str());
    }
    
    if (symbol->kind == Symbol::SymbolKind::FUNCTION) {
        std::stringstream ss;
        ss << "Cannot assign to function '" << expr->getName() << "'";
        throw TypeError(ss.str());
    }
    
    TokenType leftType = symbol->isPointer ? TokenType::POINTER : symbol->type;
    TokenType rightType = checkExpression(expr->getValue());
    
    // Be more strict about type compatibility
    if (leftType != rightType && !(leftType == TokenType::FLOAT && 
        (rightType == TokenType::INTEGER_LITERAL || rightType == TokenType::INT))) {
        std::stringstream ss;
        ss << "Cannot assign " << tokenTypeToString(rightType) 
           << " to variable of type " << tokenTypeToString(leftType);
        throw TypeError(ss.str());
    }
    
    return leftType;
}

TokenType TypeChecker::checkCall(const CallExpression* expr) {
    Symbol* symbol = symbolTable.resolve(expr->getCallee());
    if (!symbol) {
        std::stringstream ss;
        ss << "Undefined function '" << expr->getCallee() << "'";
        throw TypeError(ss.str());
    }
    
    if (symbol->kind != Symbol::SymbolKind::FUNCTION) {
        std::stringstream ss;
        ss << "'" << expr->getCallee() << "' is not a function";
        throw TypeError(ss.str());
    }
    
    const auto& params = symbol->parameters;
    const auto& args = expr->getArguments();
    
    if (params.size() != args.size()) {
        std::stringstream ss;
        ss << "Function '" << expr->getCallee() << "' expects " 
           << params.size() << " arguments, but got " << args.size();
        throw TypeError(ss.str());
    }
    
    for (size_t i = 0; i < args.size(); ++i) {
        TokenType argType = checkExpression(args[i].get());
        TokenType paramType = params[i].second;
        
        // Be more strict about argument types
        if (paramType != argType && !(paramType == TokenType::FLOAT && 
            (argType == TokenType::INTEGER_LITERAL || argType == TokenType::INT))) {
            std::stringstream ss;
            ss << "Argument " << (i + 1) << " to function '" << expr->getCallee() 
               << "' has incompatible type: expected " << tokenTypeToString(paramType) 
               << ", got " << tokenTypeToString(argType);
            throw TypeError(ss.str());
        }
    }
    
    return symbol->returnType;
}

void TypeChecker::checkStatement(const Statement* stmt) {
    if (auto* exprStmt = dynamic_cast<const ExpressionStatement*>(stmt)) {
        checkExpressionStatement(exprStmt);
    } else if (auto* blockStmt = dynamic_cast<const BlockStatement*>(stmt)) {
        checkBlock(blockStmt);
    } else if (auto* varDecl = dynamic_cast<const VariableDeclaration*>(stmt)) {
        checkVariableDeclaration(varDecl);
    } else if (auto* funcDecl = dynamic_cast<const FunctionDeclaration*>(stmt)) {
        checkFunctionDeclaration(funcDecl);
    } else if (auto* ifStmt = dynamic_cast<const IfStatement*>(stmt)) {
        checkIfStatement(ifStmt);
    } else if (auto* whileStmt = dynamic_cast<const WhileStatement*>(stmt)) {
        checkWhileStatement(whileStmt);
    } else if (auto* forStmt = dynamic_cast<const ForStatement*>(stmt)) {
        checkForStatement(forStmt);
    } else if (auto* returnStmt = dynamic_cast<const ReturnStatement*>(stmt)) {
        checkReturnStatement(returnStmt);
    } else {
        throw TypeError("Unknown statement type");
    }
}

void TypeChecker::checkExpressionStatement(const ExpressionStatement* stmt) {
    checkExpression(stmt->getExpression());
}

void TypeChecker::checkBlock(const BlockStatement* stmt) {
    symbolTable.enterScope();
    
    for (const auto& statement : stmt->getStatements()) {
        checkStatement(statement.get());
    }
    
    symbolTable.exitScope();
}

void TypeChecker::checkVariableDeclaration(const VariableDeclaration* stmt) {
    TokenType type = stmt->getType();
    bool isPointer = stmt->getIsPointer();
    std::string name = stmt->getName();
    
    // Check if already defined in any scope, not just the current one
    if (symbolTable.resolve(name)) {
        std::stringstream ss;
        ss << "Variable '" << name << "' already defined";
        throw TypeError(ss.str());
    }
    
    // Check initializer if present
    if (const Expression* initializer = stmt->getInitializer()) {
        TokenType initType = checkExpression(initializer);
        
        // Allow compatible types
        if (!isCompatibleType(type, initType)) {
            std::stringstream ss;
            ss << "Cannot initialize variable of type " 
               << tokenTypeToString(isPointer ? TokenType::POINTER : type) 
               << " with value of type " << tokenTypeToString(initType);
            throw TypeError(ss.str());
        }
    }
    
    // Add to symbol table
    Symbol symbol(name, type, isPointer, Symbol::SymbolKind::VARIABLE);
    symbolTable.define(symbol);
}

void TypeChecker::checkFunctionDeclaration(const FunctionDeclaration* stmt) {
    // Function should already be in symbol table from first pass
    Symbol* symbol = symbolTable.resolve(stmt->getName());
    if (!symbol || symbol->kind != Symbol::SymbolKind::FUNCTION) {
        throw TypeError("Internal error: function not found in symbol table");
    }
    
    // Set current function for return checking
    std::string previousFunction = currentFunctionName;
    TokenType previousReturnType = currentFunctionReturnType;
    bool previousInFunction = inFunctionBody;
    
    currentFunctionName = stmt->getName();
    currentFunctionReturnType = stmt->getReturnType();
    inFunctionBody = true;
    
    // Enter function scope
    symbolTable.enterScope();
    
    // Add parameters to symbol table
    for (const auto& param : stmt->getParameters()) {
        Symbol paramSymbol(param.first, param.second, 
                          param.second == TokenType::POINTER, 
                          Symbol::SymbolKind::PARAMETER);
        symbolTable.define(paramSymbol);
    }
    
    // Check function body
    checkStatement(stmt->getBody());
    
    // Exit function scope
    symbolTable.exitScope();
    
    // Restore previous function context
    currentFunctionName = previousFunction;
    currentFunctionReturnType = previousReturnType;
    inFunctionBody = previousInFunction;
}

void TypeChecker::checkIfStatement(const IfStatement* stmt) {
    // Check condition
    TokenType condType = checkExpression(stmt->getCondition());
    if (!isBooleanType(condType)) {
        std::stringstream ss;
        ss << "If condition must be boolean, got " << tokenTypeToString(condType);
        throw TypeError(ss.str());
    }
    
    // Check then branch
    checkStatement(stmt->getThenBranch());
    
    // Check else branch if present
    if (stmt->getElseBranch()) {
        checkStatement(stmt->getElseBranch());
    }
}

void TypeChecker::checkWhileStatement(const WhileStatement* stmt) {
    // Check condition
    TokenType condType = checkExpression(stmt->getCondition());
    if (!isBooleanType(condType)) {
        std::stringstream ss;
        ss << "While condition must be boolean, got " << tokenTypeToString(condType);
        throw TypeError(ss.str());
    }
    
    // Check body
    checkStatement(stmt->getBody());
}

void TypeChecker::checkForStatement(const ForStatement* stmt) {
    symbolTable.enterScope();
    
    // Check initializer if present
    if (stmt->getInitializer()) {
        checkStatement(stmt->getInitializer());
    }
    
    // Check condition if present
    if (stmt->getCondition()) {
        TokenType condType = checkExpression(stmt->getCondition());
        if (!isBooleanType(condType)) {
            std::stringstream ss;
            ss << "For loop condition must be boolean, got " << tokenTypeToString(condType);
            throw TypeError(ss.str());
        }
    }
    
    // Check increment if present
    if (stmt->getIncrement()) {
        checkExpression(stmt->getIncrement());
    }
    
    // Check body
    checkStatement(stmt->getBody());
    
    symbolTable.exitScope();
}

void TypeChecker::checkReturnStatement(const ReturnStatement* stmt) {
    if (!inFunctionBody) {
        throw TypeError("Return statement outside of function body");
    }
    
    // Check return value if present
    if (stmt->getValue()) {
        TokenType returnType = checkExpression(stmt->getValue());
        
        if (currentFunctionReturnType == TokenType::VOID) {
            throw TypeError("Cannot return a value from void function");
        }
        
        if (!isCompatibleType(currentFunctionReturnType, returnType)) {
            std::stringstream ss;
            ss << "Function '" << currentFunctionName << "' returns " 
               << tokenTypeToString(currentFunctionReturnType) 
               << " but got " << tokenTypeToString(returnType);
            throw TypeError(ss.str());
        }
    } else {
        // No return value
        if (currentFunctionReturnType != TokenType::VOID) {
            std::stringstream ss;
            ss << "Function '" << currentFunctionName << "' must return a value of type " 
               << tokenTypeToString(currentFunctionReturnType);
            throw TypeError(ss.str());
        }
    }
}

bool TypeChecker::isNumericType(TokenType type) const {
    return type == TokenType::INTEGER_LITERAL || 
           type == TokenType::FLOAT_LITERAL || 
           type == TokenType::INT || 
           type == TokenType::FLOAT;
}

bool TypeChecker::isBooleanType(TokenType type) const {
    return type == TokenType::BOOL || 
           type == TokenType::BOOL_LITERAL || 
           type == TokenType::TRUE || 
           type == TokenType::FALSE;
}

bool TypeChecker::isPointerType(TokenType type) const {
    return type == TokenType::POINTER;
}

bool TypeChecker::isCompatibleType(TokenType left, TokenType right) const {
    // Exact match
    if (left == right) return true;
    
    // Numeric type compatibility
    if (isNumericType(left) && isNumericType(right)) return true;
    
    // Boolean type compatibility
    if (isBooleanType(left) && isBooleanType(right)) return true;
    
    // Pointer compatibility (simplified)
    if (isPointerType(left) && right == TokenType::INTEGER_LITERAL) return true; // Allow pointer = 0 (null)
    
    return false;
}

TokenType TypeChecker::getResultType(TokenType left, TokenType op, TokenType right) const {
    // For arithmetic operators
    if (op == TokenType::PLUS || op == TokenType::MINUS || 
        op == TokenType::MULTIPLY || op == TokenType::SLASH) {
        
        if (left == TokenType::FLOAT_LITERAL || right == TokenType::FLOAT_LITERAL ||
            left == TokenType::FLOAT || right == TokenType::FLOAT) {
            return TokenType::FLOAT_LITERAL;
        }
        
        return TokenType::INTEGER_LITERAL;
    }
    
    // For comparison operators
    if (op == TokenType::EQUAL_EQUAL || op == TokenType::NOT_EQUAL ||
        op == TokenType::LESS || op == TokenType::LESS_EQUAL ||
        op == TokenType::GREATER || op == TokenType::GREATER_EQUAL) {
        return TokenType::BOOL;
    }
    
    // For logical operators
    if (op == TokenType::AND || op == TokenType::OR) {
        return TokenType::BOOL;
    }
    
    return left; // Default to left type
}

std::string TypeChecker::tokenTypeToString(TokenType type) const {
    switch (type) {
        case TokenType::INT: return "int";
        case TokenType::FLOAT: return "float";
        case TokenType::CHAR: return "char";
        case TokenType::VOID: return "void";
        case TokenType::BOOL: return "bool";
        case TokenType::STRING_LITERAL: return "string";
        case TokenType::INTEGER_LITERAL: return "int";
        case TokenType::FLOAT_LITERAL: return "float";
        case TokenType::CHAR_LITERAL: return "char";
        case TokenType::BOOL_LITERAL: return "bool";
        case TokenType::POINTER: return "pointer";
        default: return "unknown";
    }
} 