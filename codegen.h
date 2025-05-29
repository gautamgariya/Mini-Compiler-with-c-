// codegen.h
#pragma once
#include "ast.h"
#include <string>
#include <vector>
#include <memory>
#include <sstream>

enum class OpCode {
    LOAD,
    STORE,
    ADD,
    SUB,
    MUL,
    DIV,
    CMP,
    JMP,
    JE,
    JNE,
    JG,
    JL,
    CALL,
    RET,
    PUSH,
    POP,
    PRINT,
    LABEL
};

struct Instruction {
    OpCode opcode;
    std::string arg1;
    std::string arg2;
    std::string result;
    
    Instruction(OpCode op, std::string a1 = "", std::string a2 = "", std::string res = "")
        : opcode(op), arg1(std::move(a1)), arg2(std::move(a2)), result(std::move(res)) {}
};

class CodeGenerator {
private:
    std::vector<Instruction> instructions;
    int tempVarCounter;
    int labelCounter;
    std::ostringstream output;
    
    std::string generateTemp();
    std::string generateLabel();
    
    // Code generation methods for expressions
    void generateExpression(const Expression* expr);
    void generateBinaryExpression(const BinaryExpression* expr);
    void generateIdentifier(const IdentifierExpression* expr);
    void generateLiteral(const LiteralExpression* expr);
    void generateFunctionCall(const CallExpression* expr);
    
    // Code generation methods for statements
    void generateStatement(const Statement* stmt);
    void generateBlock(const BlockStatement* stmt);
    void generateVariableDeclaration(const VariableDeclaration* stmt);
    void generateFunctionDeclaration(const FunctionDeclaration* stmt);
    void generateIfStatement(const IfStatement* stmt);
    void generateWhileStatement(const WhileStatement* stmt);
    void generateForStatement(const ForStatement* stmt);
    void generateReturnStatement(const ReturnStatement* stmt);
    
public:
    CodeGenerator();
    
    void generate(const std::vector<std::unique_ptr<Statement>>& statements);
    void optimize();
    void dumpCode() const;
    const std::vector<Instruction>& getInstructions() const;
    std::string getOutput() const;
}; 