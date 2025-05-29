#pragma once

#include <memory>
#include <string>
#include <vector>
#include "lexer.h"

// Forward declarations
class Expression;
class Statement;

// Expression types
class BinaryExpression;
class UnaryExpression;
class LiteralExpression;
class IdentifierExpression;
class CallExpression;
class LogicalExpression;
class AssignExpression;

// Statement types
class ExpressionStatement;
class BlockStatement;
class IfStatement;
class WhileStatement;
class ForStatement;
class ReturnStatement;
class FunctionDeclaration;
class VariableDeclaration;

// Abstract Syntax Tree node types
enum class NodeType {
    PROGRAM,
    FUNCTION_DECL,
    VARIABLE_DECL,
    BLOCK,
    IF_STMT,
    WHILE_STMT,
    FOR_STMT,
    RETURN_STMT,
    BINARY_EXPR,
    UNARY_EXPR,
    LITERAL,
    IDENTIFIER,
    CALL_EXPR,
    ASSIGNMENT,
    PREPROCESSOR_DIRECTIVE,
    USING_DIRECTIVE
};

// Base AST node
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual NodeType getNodeType() const = 0;
};

// Base Expression class
class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

// Base Statement class
class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

// Expression node implementations
class BinaryExpression : public Expression {
    std::unique_ptr<Expression> left;
    TokenType op;
    std::unique_ptr<Expression> right;
public:
    BinaryExpression(std::unique_ptr<Expression> l, TokenType o, std::unique_ptr<Expression> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
    NodeType getNodeType() const override { return NodeType::BINARY_EXPR; }
    const Expression* getLeft() const { return left.get(); }
    const Expression* getRight() const { return right.get(); }
    TokenType getOperator() const { return op; }
};

class UnaryExpression : public Expression {
    TokenType op;
    std::unique_ptr<Expression> operand;
public:
    UnaryExpression(TokenType o, std::unique_ptr<Expression> e)
        : op(o), operand(std::move(e)) {}
    NodeType getNodeType() const override { return NodeType::UNARY_EXPR; }
    const Expression* getOperand() const { return operand.get(); }
    TokenType getOperator() const { return op; }
};

class IdentifierExpression : public Expression {
    std::string name;
public:
    IdentifierExpression(const std::string& n) : name(n) {}
    NodeType getNodeType() const override { return NodeType::IDENTIFIER; }
    const std::string& getName() const { return name; }
};

class LiteralExpression : public Expression {
    std::string value;
    TokenType literalType;
public:
    LiteralExpression(const std::string& v, TokenType type) 
        : value(v), literalType(type) {}
    NodeType getNodeType() const override { return NodeType::LITERAL; }
    const std::string& getValue() const { return value; }
    TokenType getLiteralType() const { return literalType; }
};

class CallExpression : public Expression {
    std::string callee;
    std::vector<std::unique_ptr<Expression>> arguments;
public:
    CallExpression(const std::string& name, std::vector<std::unique_ptr<Expression>> args)
        : callee(name), arguments(std::move(args)) {}
    NodeType getNodeType() const override { return NodeType::CALL_EXPR; }
    const std::string& getCallee() const { return callee; }
    const std::vector<std::unique_ptr<Expression>>& getArguments() const { return arguments; }
};

// Logical Expression (e.g., a && b, a || b)
class LogicalExpression : public Expression {
    std::unique_ptr<Expression> left;
    TokenType op;
    std::unique_ptr<Expression> right;
public:
    LogicalExpression(std::unique_ptr<Expression> left, TokenType op, std::unique_ptr<Expression> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
    NodeType getNodeType() const override { return NodeType::BINARY_EXPR; }
    const Expression* getLeft() const { return left.get(); }
    const Expression* getRight() const { return right.get(); }
    TokenType getOperator() const { return op; }
};

// Assignment Expression (e.g., a = b, a += b)
class AssignExpression : public Expression {
    std::string name;
    TokenType op;
    std::unique_ptr<Expression> value;
public:
    AssignExpression(const std::string& name, TokenType op, std::unique_ptr<Expression> value)
        : name(name), op(op), value(std::move(value)) {}
    NodeType getNodeType() const override { return NodeType::ASSIGNMENT; }
    const std::string& getName() const { return name; }
    const Expression* getValue() const { return value.get(); }
    TokenType getOperator() const { return op; }
};

// Statement node implementations
class BlockStatement : public Statement {
    std::vector<std::unique_ptr<Statement>> statements;
public:
    BlockStatement(std::vector<std::unique_ptr<Statement>> stmts)
        : statements(std::move(stmts)) {}
    NodeType getNodeType() const override { return NodeType::BLOCK; }
    const std::vector<std::unique_ptr<Statement>>& getStatements() const { return statements; }
};

class ExpressionStatement : public Statement {
    std::unique_ptr<Expression> expression;
public:
    ExpressionStatement(std::unique_ptr<Expression> expr) 
        : expression(std::move(expr)) {}
    NodeType getNodeType() const override { return NodeType::BINARY_EXPR; }
    const Expression* getExpression() const { return expression.get(); }
};

class IfStatement : public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenBranch;
    std::unique_ptr<Statement> elseBranch;
public:
    IfStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> then,
               std::unique_ptr<Statement> else_branch = nullptr)
        : condition(std::move(cond)), thenBranch(std::move(then)), elseBranch(std::move(else_branch)) {}
    NodeType getNodeType() const override { return NodeType::IF_STMT; }
    const Expression* getCondition() const { return condition.get(); }
    const Statement* getThenBranch() const { return thenBranch.get(); }
    const Statement* getElseBranch() const { return elseBranch.get(); }
};

class WhileStatement : public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;
public:
    WhileStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    NodeType getNodeType() const override { return NodeType::WHILE_STMT; }
    const Expression* getCondition() const { return condition.get(); }
    const Statement* getBody() const { return body.get(); }
};

class ForStatement : public Statement {
    std::unique_ptr<Statement> initializer;
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> increment;
    std::unique_ptr<Statement> body;
public:
    ForStatement(std::unique_ptr<Statement> init, std::unique_ptr<Expression> cond,
                std::unique_ptr<Expression> inc, std::unique_ptr<Statement> b)
        : initializer(std::move(init)), condition(std::move(cond)),
          increment(std::move(inc)), body(std::move(b)) {}
    NodeType getNodeType() const override { return NodeType::FOR_STMT; }
    const Statement* getInitializer() const { return initializer.get(); }
    const Expression* getCondition() const { return condition.get(); }
    const Expression* getIncrement() const { return increment.get(); }
    const Statement* getBody() const { return body.get(); }
};

class ReturnStatement : public Statement {
    std::unique_ptr<Expression> value;
public:
    ReturnStatement(std::unique_ptr<Expression> v = nullptr) : value(std::move(v)) {}
    NodeType getNodeType() const override { return NodeType::RETURN_STMT; }
    const Expression* getValue() const { return value.get(); }
};

class VariableDeclaration : public Statement {
    TokenType type;
    bool isPointer;
    std::string name;
    std::unique_ptr<Expression> initializer;
public:
    VariableDeclaration(TokenType t, bool ptr, const std::string& n, std::unique_ptr<Expression> init = nullptr)
        : type(t), isPointer(ptr), name(n), initializer(std::move(init)) {}
    NodeType getNodeType() const override { return NodeType::VARIABLE_DECL; }
    TokenType getType() const { return type; }
    bool getIsPointer() const { return isPointer; }
    const std::string& getName() const { return name; }
    const Expression* getInitializer() const { return initializer.get(); }
};

class FunctionDeclaration : public Statement {
    std::string name;
    TokenType returnType;
    std::vector<std::pair<std::string, TokenType>> parameters;
    std::unique_ptr<Statement> body;
public:
    FunctionDeclaration(const std::string& n, TokenType rt,
                       std::vector<std::pair<std::string, TokenType>> params,
                       std::unique_ptr<Statement> b)
        : name(n), returnType(rt), parameters(std::move(params)), body(std::move(b)) {}
    NodeType getNodeType() const override { return NodeType::FUNCTION_DECL; }
    const std::string& getName() const { return name; }
    TokenType getReturnType() const { return returnType; }
    const std::vector<std::pair<std::string, TokenType>>& getParameters() const { return parameters; }
    const Statement* getBody() const { return body.get(); }
}; 