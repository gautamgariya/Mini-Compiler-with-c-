#include "../include/symboltable.h"

bool Scope::define(const Symbol& symbol) {
    if (contains(symbol.name)) {
        return false;
    }
    
    symbols[symbol.name] = symbol;
    return true;
}

Symbol* Scope::resolve(const std::string& name) {
    if (contains(name)) {
        return &symbols[name];
    }
    return nullptr;
}

bool Scope::contains(const std::string& name) const {
    return symbols.find(name) != symbols.end();
}

SymbolTable::SymbolTable() {
    // Create global scope
    enterScope();
}

void SymbolTable::enterScope() {
    scopes.push_back(std::make_unique<Scope>());
}

void SymbolTable::exitScope() {
    if (scopes.size() > 1) { // Keep at least one scope (global)
        scopes.pop_back();
    }
}

bool SymbolTable::define(const Symbol& symbol) {
    return scopes.back()->define(symbol);
}

Symbol* SymbolTable::resolve(const std::string& name) {
    // Search from innermost to outermost scope
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (Symbol* symbol = (*it)->resolve(name)) {
            return symbol;
        }
    }
    return nullptr;
}

Symbol* SymbolTable::resolveLocal(const std::string& name) {
    // Only look in current scope
    return scopes.back()->resolve(name);
}

bool SymbolTable::isGlobalScope() const {
    return scopes.size() == 1;
} 