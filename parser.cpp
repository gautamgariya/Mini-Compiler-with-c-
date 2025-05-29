#include "../include/parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(Lexer& lexer) : lexer(lexer) {
    advance(); // Get first token
}

void Parser::advance() {
    currentToken = lexer.getNextToken();
}

void Parser::synchronize() {
    advance();
    
    while (!check(TokenType::END_OF_FILE)) {
        if (currentToken.type == TokenType::SEMICOLON ||
            currentToken.type == TokenType::RBRACE) {
            advance();
            return;
        }
        
        switch (currentToken.type) {
            case TokenType::INT:
            case TokenType::FLOAT:
            case TokenType::CHAR:
            case TokenType::VOID:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::FOR:
            case TokenType::RETURN:
                return;
            default:
                advance();
        }
    }
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    return currentToken.type == type;
}

void Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        advance();
        return;
    }
    throw std::runtime_error(message);
}

Token Parser::peek() const {
    return currentToken;
}

std::unique_ptr<Expression> Parser::parseExpression() {
    try {
        return parseAssignment();
    } catch (const std::runtime_error& e) {
        synchronize();
        throw;
    }
}

std::unique_ptr<Expression> Parser::parseAssignment() {
    try {
        auto expr = parseLogicalOr();

        // Handle assignment operators
        if (check(TokenType::EQUAL) || check(TokenType::PLUS_EQUAL) ||
            check(TokenType::MINUS_EQUAL) || check(TokenType::MULTIPLY_EQUAL) ||
            check(TokenType::DIVIDE_EQUAL)) {
            TokenType op = currentToken.type;
            advance();
            
            // For compound assignments (+=, -=, etc.), we need to create a binary expression
            if (op != TokenType::EQUAL) {
                if (auto* varExpr = dynamic_cast<IdentifierExpression*>(expr.get())) {
                    auto right = parseAssignment();
                    if (!right) {
                        throw std::runtime_error("Expect expression after operator.");
                    }
                    
                    auto binaryOp = (op == TokenType::PLUS_EQUAL) ? TokenType::PLUS :
                                  (op == TokenType::MINUS_EQUAL) ? TokenType::MINUS :
                                  (op == TokenType::MULTIPLY_EQUAL) ? TokenType::MULTIPLY :
                                  TokenType::SLASH;
                    
                    auto binaryExpr = std::make_unique<BinaryExpression>(
                        std::make_unique<IdentifierExpression>(varExpr->getName()),
                        binaryOp,
                        std::move(right)
                    );
                    return std::make_unique<AssignExpression>(varExpr->getName(), TokenType::EQUAL, std::move(binaryExpr));
                }
            } else {
                if (auto* varExpr = dynamic_cast<IdentifierExpression*>(expr.get())) {
                    auto value = parseAssignment();
                    if (!value) {
                        throw std::runtime_error("Expect expression after '='.");
                    }
                    return std::make_unique<AssignExpression>(varExpr->getName(), op, std::move(value));
                }
            }
            
            throw std::runtime_error("Invalid assignment target.");
        }

        // Handle increment/decrement operators
        if (check(TokenType::INCREMENT) || check(TokenType::DECREMENT)) {
            if (auto* varExpr = dynamic_cast<IdentifierExpression*>(expr.get())) {
                TokenType op = currentToken.type;
                advance();
                return std::make_unique<UnaryExpression>(op, std::make_unique<IdentifierExpression>(varExpr->getName()));
            }
            throw std::runtime_error("Invalid increment/decrement target.");
        }

        return expr;
    } catch (const std::runtime_error& e) {
        synchronize();
        throw;
    }
}

std::unique_ptr<Expression> Parser::parseLogicalOr() {
    try {
        auto expr = parseLogicalAnd();

        while (check(TokenType::OR)) {
            TokenType op = currentToken.type;
            advance();
            auto right = parseLogicalAnd();
            expr = std::make_unique<LogicalExpression>(std::move(expr), op, std::move(right));
        }

        return expr;
    } catch (const std::runtime_error& e) {
        synchronize();
        throw;
    }
}

std::unique_ptr<Expression> Parser::parseLogicalAnd() {
    try {
        auto expr = parseEquality();

        while (check(TokenType::AND)) {
            TokenType op = currentToken.type;
            advance();
            auto right = parseEquality();
            expr = std::make_unique<LogicalExpression>(std::move(expr), op, std::move(right));
        }

        return expr;
    } catch (const std::runtime_error& e) {
        synchronize();
        throw;
    }
}

std::unique_ptr<Expression> Parser::parseEquality() {
    try {
        auto expr = parseComparison();

        while (check(TokenType::EQUAL_EQUAL) || check(TokenType::NOT_EQUAL)) {
            TokenType op = currentToken.type;
            advance();
            auto right = parseComparison();
            expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
        }

        return expr;
    } catch (const std::runtime_error& e) {
        synchronize();
        throw;
    }
}

std::unique_ptr<Expression> Parser::parseComparison() {
    try {
        auto expr = parseTerm();

        while (check(TokenType::LESS) || check(TokenType::LESS_EQUAL) ||
               check(TokenType::GREATER) || check(TokenType::GREATER_EQUAL)) {
            TokenType op = currentToken.type;
            advance();
            auto right = parseTerm();
            expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
        }

        return expr;
    } catch (const std::runtime_error& e) {
        synchronize();
        throw;
    }
}

std::unique_ptr<Expression> Parser::parseTerm() {
    try {
        auto expr = parseFactor();

        while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
            TokenType op = currentToken.type;
            advance();
            auto right = parseFactor();
            expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
        }

        return expr;
    } catch (const std::runtime_error& e) {
        synchronize();
        throw;
    }
}

std::unique_ptr<Expression> Parser::parseFactor() {
    try {
        auto expr = parsePrimary();

        while (check(TokenType::MULTIPLY) || check(TokenType::SLASH)) {
            TokenType op = currentToken.type;
            advance();
            auto right = parsePrimary();
            expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
        }

        return expr;
    } catch (const std::runtime_error& e) {
        synchronize();
        throw;
    }
}

std::unique_ptr<Expression> Parser::parsePrimary() {
    try {
        // Handle unary operators
        if (check(TokenType::NOT) || check(TokenType::MULTIPLY) || check(TokenType::AMPERSAND) ||
            check(TokenType::INCREMENT) || check(TokenType::DECREMENT) ||
            check(TokenType::PLUS) || check(TokenType::MINUS)) {
            TokenType op = currentToken.type;
            advance();
            auto expr = parsePrimary();
            if (!expr) {
                throw std::runtime_error("Expect expression after unary operator.");
            }
            return std::make_unique<UnaryExpression>(op, std::move(expr));
        }

        if (check(TokenType::TRUE)) {
            advance();
            return std::make_unique<LiteralExpression>("true", TokenType::BOOL_LITERAL);
        }
        if (check(TokenType::FALSE)) {
            advance();
            return std::make_unique<LiteralExpression>("false", TokenType::BOOL_LITERAL);
        }

        // Handle literals
        if (check(TokenType::INTEGER_LITERAL) || check(TokenType::FLOAT_LITERAL)) {
            std::string value = currentToken.lexeme;
            TokenType type = currentToken.type;
            advance();
            return std::make_unique<LiteralExpression>(value, type);
        }
        
        // Handle character literals
        if (check(TokenType::CHAR_LITERAL)) {
            std::string value = currentToken.lexeme;
            // Remove quotes and process escape sequences
            if (value.length() >= 2) {
                value = value.substr(1, value.length() - 2);
                if (value[0] == '\\' && value.length() > 1) {
                    switch (value[1]) {
                        case 'n': value = "\n"; break;
                        case 't': value = "\t"; break;
                        case '\\': value = "\\"; break;
                        case '\'': value = "'"; break;
                        case '"': value = "\""; break;
                        default: value = value.substr(1);
                    }
                }
            }
            advance();
            return std::make_unique<LiteralExpression>(value, TokenType::CHAR_LITERAL);
        }
        
        // Handle string literals
        if (check(TokenType::STRING_LITERAL)) {
            std::string value = currentToken.lexeme;
            // Remove quotes and process escape sequences
            if (value.length() >= 2) {
                value = value.substr(1, value.length() - 2);
                std::string processed;
                for (size_t i = 0; i < value.length(); i++) {
                    if (value[i] == '\\' && i + 1 < value.length()) {
                        i++;
                        switch (value[i]) {
                            case 'n': processed += '\n'; break;
                            case 't': processed += '\t'; break;
                            case '\\': processed += '\\'; break;
                            case '\'': processed += '\''; break;
                            case '"': processed += '"'; break;
                            default: processed += value[i];
                        }
                    } else {
                        processed += value[i];
                    }
                }
                value = processed;
            }
            advance();
            return std::make_unique<LiteralExpression>(value, TokenType::STRING_LITERAL);
        }
        
        if (check(TokenType::IDENTIFIER)) {
            std::string name = currentToken.lexeme;
            advance();
            
            // Handle post-increment/decrement
            if (check(TokenType::INCREMENT) || check(TokenType::DECREMENT)) {
                TokenType op = currentToken.type;
                advance();
                return std::make_unique<UnaryExpression>(op, std::make_unique<IdentifierExpression>(name));
            }
            
            // Function call
            if (check(TokenType::LPAREN)) {
                advance(); // consume '('
                std::vector<std::unique_ptr<Expression>> arguments;
                
                // Handle empty argument list
                if (!check(TokenType::RPAREN)) {
                    do {
                        try {
                            if (check(TokenType::COMMA)) {
                                advance();
                                continue;
                            }
                            
                            auto arg = parseExpression();
                            if (!arg) {
                                throw std::runtime_error("Invalid function argument.");
                            }
                            arguments.push_back(std::move(arg));
                        } catch (const std::runtime_error& e) {
                            // Skip to next argument or closing parenthesis
                            while (!check(TokenType::RPAREN) && !check(TokenType::COMMA) && 
                                   !check(TokenType::END_OF_FILE)) {
                                advance();
                            }
                            if (check(TokenType::COMMA)) {
                                advance();
                            }
                        }
                    } while (!check(TokenType::RPAREN) && !check(TokenType::END_OF_FILE));
                }
                
                consume(TokenType::RPAREN, "Expect ')' after arguments.");
                return std::make_unique<CallExpression>(name, std::move(arguments));
            }
            
            // Handle stream operators
            if (check(TokenType::LEFT_SHIFT) || check(TokenType::RIGHT_SHIFT)) {
                std::unique_ptr<Expression> left = std::make_unique<IdentifierExpression>(name);
                
                do {
                    TokenType op = currentToken.type;
                    advance();
                    
                    // Handle endl as a special case
                    if (check(TokenType::ENDL)) {
                        advance();
                        auto right = std::make_unique<IdentifierExpression>("endl");
                        left = std::make_unique<BinaryExpression>(std::move(left), op, std::move(right));
                        continue;
                    }
                    
                    auto right = parseExpression();
                    if (!right) {
                        throw std::runtime_error("Expect expression after stream operator.");
                    }
                    left = std::make_unique<BinaryExpression>(std::move(left), op, std::move(right));
                } while (check(TokenType::LEFT_SHIFT) || check(TokenType::RIGHT_SHIFT));
                
                return left;
            }
            
            return std::make_unique<IdentifierExpression>(name);
        }
        
        if (check(TokenType::LPAREN)) {
            advance();
            auto expr = parseExpression();
            if (!expr) {
                throw std::runtime_error("Expect expression inside parentheses.");
            }
            consume(TokenType::RPAREN, "Expect ')' after expression.");
            return expr;
        }
        
        throw std::runtime_error("Expect expression.");
    } catch (const std::runtime_error& e) {
        synchronize();
        throw;
    }
}

std::unique_ptr<Statement> Parser::parseStatement() {
    try {
        // Skip any extra semicolons
        while (match(TokenType::SEMICOLON));
        
        if (check(TokenType::HASH)) {
            return parsePreprocessorDirective();
        }

        if (check(TokenType::USING)) {
            return parseUsingDirective();
        }

        if (check(TokenType::RETURN)) {
            advance(); // consume 'return'
            std::unique_ptr<Expression> value = nullptr;
            if (!check(TokenType::SEMICOLON)) {
                value = parseExpression();
            }
            consume(TokenType::SEMICOLON, "Expect ';' after return value.");
            return std::make_unique<ReturnStatement>(std::move(value));
        }

        if (check(TokenType::IF)) {
            advance(); // consume 'if'
            consume(TokenType::LPAREN, "Expect '(' after 'if'.");
            auto condition = parseExpression();
            consume(TokenType::RPAREN, "Expect ')' after if condition.");
            
            consume(TokenType::LBRACE, "Expect '{' before if body.");
            auto thenBranch = parseBlock();
            consume(TokenType::RBRACE, "Expect '}' after if body.");
            
            std::unique_ptr<Statement> elseBranch = nullptr;
            if (match(TokenType::ELSE)) {
                consume(TokenType::LBRACE, "Expect '{' before else body.");
                elseBranch = parseBlock();
                consume(TokenType::RBRACE, "Expect '}' after else body.");
            }
            
            return std::make_unique<IfStatement>(std::move(condition), 
                                               std::move(thenBranch),
                                               std::move(elseBranch));
        }

        if (check(TokenType::WHILE)) {
            advance(); // consume 'while'
            consume(TokenType::LPAREN, "Expect '(' after 'while'.");
            auto condition = parseExpression();
            consume(TokenType::RPAREN, "Expect ')' after condition.");
            
            consume(TokenType::LBRACE, "Expect '{' before while body.");
            auto body = parseBlock();
            consume(TokenType::RBRACE, "Expect '}' after while body.");
            
            return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
        }

        if (check(TokenType::FOR)) {
            advance(); // consume 'for'
            consume(TokenType::LPAREN, "Expect '(' after 'for'.");
            
            // Initializer
            std::unique_ptr<Statement> initializer;
            if (!check(TokenType::SEMICOLON)) {
                if (check(TokenType::INT) || check(TokenType::FLOAT) || check(TokenType::CHAR) ||
                    check(TokenType::BOOL) || check(TokenType::STRING_LITERAL) ||
                    (check(TokenType::IDENTIFIER) && currentToken.lexeme == "string")) {
                    initializer = parseFunctionOrVariableDeclaration();
                } else {
                    auto expr = parseExpression();
                    consume(TokenType::SEMICOLON, "Expect ';' after for initializer.");
                    initializer = std::make_unique<ExpressionStatement>(std::move(expr));
                }
            } else {
                advance(); // consume ';'
            }
            
            // Condition
            std::unique_ptr<Expression> condition;
            if (!check(TokenType::SEMICOLON)) {
                condition = parseExpression();
            }
            consume(TokenType::SEMICOLON, "Expect ';' after for condition.");
            
            // Increment
            std::unique_ptr<Expression> increment;
            if (!check(TokenType::RPAREN)) {
                increment = parseExpression();
            }
            consume(TokenType::RPAREN, "Expect ')' after for clauses.");
            
            consume(TokenType::LBRACE, "Expect '{' before for body.");
            auto body = parseBlock();
            consume(TokenType::RBRACE, "Expect '}' after for body.");
            
            return std::make_unique<ForStatement>(std::move(initializer), std::move(condition),
                                                std::move(increment), std::move(body));
        }
        
        // Type declarations
        if (check(TokenType::INT) || check(TokenType::FLOAT) || check(TokenType::CHAR) || 
            check(TokenType::VOID) || check(TokenType::BOOL) || check(TokenType::STRING_LITERAL) ||
            (check(TokenType::IDENTIFIER) && currentToken.lexeme == "string")) {
            return parseFunctionOrVariableDeclaration();
        }

        // Handle expression statement
        auto expr = parseExpression();
        consume(TokenType::SEMICOLON, "Expect ';' after expression.");
        return std::make_unique<ExpressionStatement>(std::move(expr));
    } catch (const std::runtime_error& e) {
        synchronize();
        throw;
    }
}

std::unique_ptr<Statement> Parser::parseBlock() {
    std::vector<std::unique_ptr<Statement>> statements;
    
    while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
        statements.push_back(parseStatement());
    }
    
    return std::make_unique<BlockStatement>(std::move(statements));
}

std::unique_ptr<Statement> Parser::parseFunctionOrVariableDeclaration() {
    try {
        TokenType type = currentToken.type;
        if (type == TokenType::IDENTIFIER && currentToken.lexeme == "string") {
            type = TokenType::STRING_LITERAL;
        }
        advance();
        
        // Handle pointer types
        bool isPointer = false;
        if (check(TokenType::MULTIPLY)) {
            isPointer = true;
            advance();
        }
        
        if (!check(TokenType::IDENTIFIER)) {
            throw std::runtime_error("Expect identifier after type.");
        }
        
        std::string name = currentToken.lexeme;
        advance();

        // Function declaration
        if (check(TokenType::LPAREN)) {
            advance(); // consume '('
            std::vector<std::pair<std::string, TokenType>> parameters;
            
            if (!check(TokenType::RPAREN)) {
                do {
                    TokenType paramType = currentToken.type;
                    if (paramType == TokenType::IDENTIFIER && currentToken.lexeme == "string") {
                        paramType = TokenType::STRING_LITERAL;
                    }
                    advance();
                    
                    bool isParamPointer = false;
                    if (check(TokenType::MULTIPLY)) {
                        isParamPointer = true;
                        advance();
                    }
                    
                    if (!check(TokenType::IDENTIFIER)) {
                        throw std::runtime_error("Expect parameter name.");
                    }
                    std::string paramName = currentToken.lexeme;
                    advance();
                    
                    parameters.push_back(std::make_pair(paramName, isParamPointer ? TokenType::POINTER : paramType));
                } while (match(TokenType::COMMA));
            }
            
            consume(TokenType::RPAREN, "Expect ')' after parameters.");
            consume(TokenType::LBRACE, "Expect '{' before function body.");
            
            auto body = parseBlock();
            consume(TokenType::RBRACE, "Expect '}' after function body.");
            
            return std::make_unique<FunctionDeclaration>(name, type, std::move(parameters), std::move(body));
        }

        // Variable declaration
        std::vector<std::unique_ptr<Statement>> declarations;
        
        // Handle first variable
        std::unique_ptr<Expression> initializer = nullptr;
        if (check(TokenType::EQUAL)) {
            advance();
            try {
                initializer = parseExpression();
                if (!initializer) {
                    throw std::runtime_error("Invalid initializer expression.");
                }
            } catch (const std::runtime_error& e) {
                synchronize();
                throw;
            }
        }
        
        declarations.push_back(std::make_unique<VariableDeclaration>(type, isPointer, name, std::move(initializer)));
        
        // Handle multiple declarations (e.g., int a = 1, b = 2, c;)
        while (match(TokenType::COMMA)) {
            if (!check(TokenType::IDENTIFIER)) {
                throw std::runtime_error("Expect identifier after ','.");
            }
            
            std::string nextName = currentToken.lexeme;
            advance();
            
            std::unique_ptr<Expression> nextInitializer = nullptr;
            if (check(TokenType::EQUAL)) {
                advance();
                try {
                    nextInitializer = parseExpression();
                    if (!nextInitializer) {
                        throw std::runtime_error("Invalid initializer expression.");
                    }
                } catch (const std::runtime_error& e) {
                    synchronize();
                    throw;
                }
            }
            
            declarations.push_back(std::make_unique<VariableDeclaration>(type, isPointer, nextName, std::move(nextInitializer)));
        }
        
        if (!check(TokenType::SEMICOLON)) {
            throw std::runtime_error("Expect ';' after variable declaration.");
        }
        advance(); // consume semicolon
        
        if (declarations.size() == 1) {
            return std::move(declarations[0]);
        }
        
        return std::make_unique<BlockStatement>(std::move(declarations));
    } catch (const std::runtime_error& e) {
        synchronize();
        throw;
    }
}

std::unique_ptr<Statement> Parser::parsePreprocessorDirective() {
    advance(); // consume #
    if (match(TokenType::INCLUDE)) {
        std::string header;
        if (match(TokenType::LESS)) {
            // Handle system header
            while (!check(TokenType::GREATER) && !check(TokenType::END_OF_FILE)) {
                header += currentToken.lexeme;
                advance();
            }
            consume(TokenType::GREATER, "Expect '>' after include path");
        } else if (match(TokenType::STRING_LITERAL)) {
            // Handle local header
            header = currentToken.lexeme;
        } else {
            throw std::runtime_error("Expect header name after #include");
        }
        return std::make_unique<ExpressionStatement>(
            std::make_unique<LiteralExpression>(header, TokenType::STRING_LITERAL));
    }
    throw std::runtime_error("Unsupported preprocessor directive");
}

std::unique_ptr<Statement> Parser::parseUsingDirective() {
    advance(); // consume 'using'
    if (match(TokenType::NAMESPACE)) {
        if (match(TokenType::STD)) {
            consume(TokenType::SEMICOLON, "Expect ';' after namespace std");
            return std::make_unique<ExpressionStatement>(std::make_unique<IdentifierExpression>("using_namespace_std"));
        }
    }
    throw std::runtime_error("Unsupported using directive");
}

std::vector<std::unique_ptr<Statement>> Parser::parse() {
    std::vector<std::unique_ptr<Statement>> statements;
    
    while (!check(TokenType::END_OF_FILE)) {
        try {
            if (check(TokenType::SEMICOLON)) {
                advance(); // Skip extra semicolons
                continue;
            }
            
            auto stmt = parseStatement();
            if (stmt) {
                statements.push_back(std::move(stmt));
            }
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            synchronize();
        }
    }
    
    return statements;
} 