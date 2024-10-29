//
// Created by fathi on 10/20/2024.
//

#ifndef TIGE_SYMBOL_TABLE_H
#define TIGE_SYMBOL_TABLE_H

#include "symbol.h"

#define HASH_TABLE_SIZE 256

typedef struct Scope {
    Symbol* hash_table[HASH_TABLE_SIZE];
    size_t capacity;
    struct Scope* parent;
    uint16_t variable_index_counter;
} Scope;

typedef struct SymbolTable {
    Scope* current_scope;
    // the current nesting level
    uint8_t level;
} SymbolTable;

// Create a new symbol table (with global scope)
SymbolTable* create_symbol_table();

// Destroy the symbol table and all its scopes
void destroy_symbol_table(SymbolTable* table);

// Enter a new scope
void enter_scope(SymbolTable* table);

// Exit the current scope
void exit_scope(SymbolTable* table);

// Add a symbol to the current scope
// Returns the index of the inserted element on success, -1 on failure
int64_t add_symbol(SymbolTable* table, const char* name, SymbolType type);

// Lookup a symbol by name (searches from current scope upwards)
// Returns a pointer to the Symbol if found, NULL otherwise
Symbol* lookup_symbol(SymbolTable* table, const char* name);

Symbol* lookup_symbol_ndx(SymbolTable* table, uint64_t index);

#endif //TIGE_SYMBOL_TABLE_H
