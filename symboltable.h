#pragma once

#include "token.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

class Symbol {
public:
    enum class SymbolKind {
        VARIABLE,
        FUNCTION,
        PARAMETER
    };
    
    std::string name;
    TokenType type;
    bool isPointer;
    SymbolKind kind;
    
    // For functions
    TokenType returnType;
    std::vector<std::pair<std::string, TokenType>> parameters;
    
    Symbol() : isPointer(false), kind(SymbolKind::VARIABLE) {}
    
    Symbol(std::string name, TokenType type, bool isPointer, SymbolKind kind)
        : name(std::move(name)), type(type), isPointer(isPointer), kind(kind) {}
    
    // Constructor for functions
    Symbol(std::string name, TokenType returnType, std::vector<std::pair<std::string, TokenType>> parameters)
        : name(std::move(name)), returnType(returnType), parameters(std::move(parameters)), 
          kind(SymbolKind::FUNCTION), isPointer(false) {}
};

class Scope {
private:
    std::unordered_map<std::string, Symbol> symbols;
    
public:
    bool define(const Symbol& symbol);
    Symbol* resolve(const std::string& name);
    bool contains(const std::string& name) const;
};

class SymbolTable {
private:
    std::vector<std::unique_ptr<Scope>> scopes;
    
public:
    SymbolTable();
    
    void enterScope();
    void exitScope();
    
    bool define(const Symbol& symbol);
    Symbol* resolve(const std::string& name);
    Symbol* resolveLocal(const std::string& name);
    bool isGlobalScope() const;
}; 