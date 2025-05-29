#include "../include/codegen.h"
#include <iostream>
#include <sstream>

CodeGenerator::CodeGenerator()
    : tempVarCounter(0), labelCounter(0) {}

std::string CodeGenerator::generateTemp() {
    return "t" + std::to_string(++tempVarCounter);
}

std::string CodeGenerator::generateLabel() {
    return "L" + std::to_string(++labelCounter);
}

void CodeGenerator::generateExpression(const Expression* expr) {
    if (auto* binaryExpr = dynamic_cast<const BinaryExpression*>(expr)) {
        generateBinaryExpression(binaryExpr);
    } else if (auto* identifierExpr = dynamic_cast<const IdentifierExpression*>(expr)) {
        generateIdentifier(identifierExpr);
    } else if (auto* literalExpr = dynamic_cast<const LiteralExpression*>(expr)) {
        generateLiteral(literalExpr);
    } else if (auto* callExpr = dynamic_cast<const CallExpression*>(expr)) {
        generateFunctionCall(callExpr);
    } else {
        // Handle other expression types
        std::cerr << "Warning: Unsupported expression type" << std::endl;
    }
}

void CodeGenerator::generateStatement(const Statement* stmt) {
    if (auto* varDecl = dynamic_cast<const VariableDeclaration*>(stmt)) {
        generateVariableDeclaration(varDecl);
    } else if (auto* funcDecl = dynamic_cast<const FunctionDeclaration*>(stmt)) {
        generateFunctionDeclaration(funcDecl);
    } else if (auto* blockStmt = dynamic_cast<const BlockStatement*>(stmt)) {
        generateBlock(blockStmt);
    } else if (auto* ifStmt = dynamic_cast<const IfStatement*>(stmt)) {
        generateIfStatement(ifStmt);
    } else if (auto* whileStmt = dynamic_cast<const WhileStatement*>(stmt)) {
        generateWhileStatement(whileStmt);
    } else if (auto* forStmt = dynamic_cast<const ForStatement*>(stmt)) {
        generateForStatement(forStmt);
    } else if (auto* returnStmt = dynamic_cast<const ReturnStatement*>(stmt)) {
        generateReturnStatement(returnStmt);
    } else if (auto* exprStmt = dynamic_cast<const ExpressionStatement*>(stmt)) {
        generateExpression(exprStmt->getExpression());
    } else {
        std::cerr << "Warning: Unsupported statement type" << std::endl;
    }
}

void CodeGenerator::generate(const std::vector<std::unique_ptr<Statement>>& statements) {
    for (const auto& stmt : statements) {
        generateStatement(stmt.get());
    }
}

const std::vector<Instruction>& CodeGenerator::getInstructions() const {
    return instructions;
}

void CodeGenerator::optimize() {
    // Basic optimization: remove redundant loads/stores
    auto it = instructions.begin();
    while (it != instructions.end()) {
        if (it->opcode == OpCode::LOAD && std::next(it) != instructions.end() &&
            std::next(it)->opcode == OpCode::STORE) {
            it = instructions.erase(it);
            it = instructions.erase(it);
        } else {
            ++it;
        }
    }
}

void CodeGenerator::dumpCode() const {
    for (const auto& instr : instructions) {
        std::cout << "  ";
        switch (instr.opcode) {
            case OpCode::LOAD:
                std::cout << "LOAD " << instr.arg1 << " -> " << instr.result;
                break;
            case OpCode::STORE:
                std::cout << "STORE " << instr.arg1 << " -> " << instr.result;
                break;
            case OpCode::ADD:
                std::cout << "ADD " << instr.arg1 << ", " << instr.arg2 << " -> " << instr.result;
                break;
            case OpCode::SUB:
                std::cout << "SUB " << instr.arg1 << ", " << instr.arg2 << " -> " << instr.result;
                break;
            case OpCode::MUL:
                std::cout << "MUL " << instr.arg1 << ", " << instr.arg2 << " -> " << instr.result;
                break;
            case OpCode::DIV:
                std::cout << "DIV " << instr.arg1 << ", " << instr.arg2 << " -> " << instr.result;
                break;
            case OpCode::CMP:
                std::cout << "CMP " << instr.arg1 << ", " << instr.arg2 << " -> " << instr.result;
                break;
            case OpCode::JMP:
                std::cout << "JMP " << instr.arg1;
                break;
            case OpCode::JE:
                std::cout << "JE " << instr.arg1;
                break;
            case OpCode::JNE:
                std::cout << "JNE " << instr.arg1;
                break;
            case OpCode::JG:
                std::cout << "JG " << instr.arg1;
                break;
            case OpCode::JL:
                std::cout << "JL " << instr.arg1;
                break;
            case OpCode::CALL:
                std::cout << "CALL " << instr.arg1;
                break;
            case OpCode::RET:
                std::cout << "RET";
                break;
            case OpCode::PUSH:
                std::cout << "PUSH " << instr.arg1;
                break;
            case OpCode::POP:
                std::cout << "POP";
                break;
            case OpCode::PRINT:
                std::cout << "PRINT " << instr.arg1;
                break;
            case OpCode::LABEL:
                std::cout << instr.arg1 << ":";
                break;
            default:
                std::cout << "Unknown instruction";
        }
        std::cout << std::endl;
    }
}

void CodeGenerator::generateVariableDeclaration(const VariableDeclaration* decl) {
    // Generate code for initializer if present
    std::string value;
    if (const Expression* init = decl->getInitializer()) {
        generateExpression(init);
        value = generateTemp();
    }
    
    // Store the value in the variable
    instructions.emplace_back(OpCode::STORE, value, "", decl->getName());
}

void CodeGenerator::generateFunctionDeclaration(const FunctionDeclaration* decl) {
    // Generate function label
    std::string label = decl->getName();
    instructions.emplace_back(OpCode::LABEL, label, "", "");
    
    // Generate code for function body
    if (const Statement* body = decl->getBody()) {
        generateStatement(body);
    }
    
    // Add return instruction if not present
    if (instructions.empty() || instructions.back().opcode != OpCode::RET) {
        instructions.emplace_back(OpCode::RET, "", "", "");
    }
}

void CodeGenerator::generateBlock(const BlockStatement* block) {
    // Generate code for each statement in the block
    for (const auto& stmt : block->getStatements()) {
        generateStatement(stmt.get());
    }
}

void CodeGenerator::generateIfStatement(const IfStatement* ifStmt) {
    std::string elseLabel = generateLabel();
    std::string endLabel = generateLabel();
    
    // Generate condition code
    generateExpression(ifStmt->getCondition());
    instructions.emplace_back(OpCode::JE, elseLabel);
    
    // Generate then branch
    generateStatement(ifStmt->getThenBranch());
    instructions.emplace_back(OpCode::JMP, endLabel);
    
    // Generate else branch if present
    instructions.emplace_back(OpCode::LABEL, elseLabel);
    if (const Statement* elseBranch = ifStmt->getElseBranch()) {
        generateStatement(elseBranch);
    }
    
    instructions.emplace_back(OpCode::LABEL, endLabel);
}

void CodeGenerator::generateWhileStatement(const WhileStatement* whileStmt) {
    std::string startLabel = generateLabel();
    std::string endLabel = generateLabel();
    
    // Generate loop header
    instructions.emplace_back(OpCode::LABEL, startLabel);
    
    // Generate condition code
    generateExpression(whileStmt->getCondition());
    instructions.emplace_back(OpCode::JE, endLabel);
    
    // Generate loop body
    generateStatement(whileStmt->getBody());
    instructions.emplace_back(OpCode::JMP, startLabel);
    
    // Generate loop end
    instructions.emplace_back(OpCode::LABEL, endLabel);
}

void CodeGenerator::generateForStatement(const ForStatement* forStmt) {
    std::string startLabel = generateLabel();
    std::string endLabel = generateLabel();
    
    // Generate initializer
    if (const Statement* init = forStmt->getInitializer()) {
        generateStatement(init);
    }
    
    // Generate loop header
    instructions.emplace_back(OpCode::LABEL, startLabel);
    
    // Generate condition code
    if (const Expression* cond = forStmt->getCondition()) {
        generateExpression(cond);
        instructions.emplace_back(OpCode::JE, endLabel);
    }
    
    // Generate loop body
    generateStatement(forStmt->getBody());
    
    // Generate increment
    if (const Expression* inc = forStmt->getIncrement()) {
        generateExpression(inc);
    }
    
    instructions.emplace_back(OpCode::JMP, startLabel);
    instructions.emplace_back(OpCode::LABEL, endLabel);
}

void CodeGenerator::generateReturnStatement(const ReturnStatement* returnStmt) {
    // Generate code for return value if present
    if (const Expression* value = returnStmt->getValue()) {
        generateExpression(value);
    }
    
    instructions.emplace_back(OpCode::RET);
}

void CodeGenerator::generateBinaryExpression(const BinaryExpression* expr) {
    // Generate code for operands
    generateExpression(expr->getLeft());
    std::string leftTemp = generateTemp();
    
    generateExpression(expr->getRight());
    std::string rightTemp = generateTemp();
    
    // Generate operation
    std::string resultTemp = generateTemp();
    switch (expr->getOperator()) {
        case TokenType::PLUS:
            instructions.emplace_back(OpCode::ADD, leftTemp, rightTemp, resultTemp);
            break;
        case TokenType::MINUS:
            instructions.emplace_back(OpCode::SUB, leftTemp, rightTemp, resultTemp);
            break;
        case TokenType::MULTIPLY:
            instructions.emplace_back(OpCode::MUL, leftTemp, rightTemp, resultTemp);
            break;
        case TokenType::SLASH:
            instructions.emplace_back(OpCode::DIV, leftTemp, rightTemp, resultTemp);
            break;
        case TokenType::EQUAL_EQUAL:
        case TokenType::NOT_EQUAL:
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
            instructions.emplace_back(OpCode::CMP, leftTemp, rightTemp, resultTemp);
            break;
        default:
            std::cerr << "Warning: Unsupported binary operator" << std::endl;
            break;
    }
}

void CodeGenerator::generateIdentifier(const IdentifierExpression* expr) {
    std::string temp = generateTemp();
    instructions.emplace_back(OpCode::LOAD, expr->getName(), "", temp);
}

void CodeGenerator::generateLiteral(const LiteralExpression* expr) {
    std::string temp = generateTemp();
    instructions.emplace_back(OpCode::STORE, expr->getValue(), "", temp);
}

void CodeGenerator::generateFunctionCall(const CallExpression* expr) {
    std::vector<std::string> argTemps;
    
    // Generate code for arguments
    for (const auto& arg : expr->getArguments()) {
        generateExpression(arg.get());
        std::string temp = generateTemp();
        argTemps.push_back(temp);
    }
    
    // Push arguments
    for (const auto& temp : argTemps) {
        instructions.emplace_back(OpCode::PUSH, temp, "", "");
    }
    
    // Generate call
    instructions.emplace_back(OpCode::CALL, expr->getCallee(), "", "");
    
    // Pop arguments
    for (size_t i = 0; i < argTemps.size(); ++i) {
        instructions.emplace_back(OpCode::POP, "", "", "");
    }
    
    // Store return value
    std::string resultTemp = generateTemp();
    instructions.emplace_back(OpCode::STORE, "retval", "", resultTemp);
}

std::string CodeGenerator::getOutput() const {
    return output.str();
} 