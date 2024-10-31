#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// djb2 algorithm
static unsigned long hash(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned  char)*str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash % HASH_TABLE_SIZE;
}

// Create a new scope
static Scope* create_scope(Scope* parent) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    if (!scope) {
        fprintf(stderr, "Error: Memory allocation failed for Scope.\n");
        exit(1);
    }
    scope->capacity = HASH_TABLE_SIZE;
    memset(scope->hash_table, 0, HASH_TABLE_SIZE * sizeof(Symbol*));

    scope->parent = parent;
    scope->variable_index_counter = parent ? parent->variable_index_counter + 1 : 0;
    return scope;
}

// Create a new symbol table with a global scope
SymbolTable* create_symbol_table() {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (!table) {
        fprintf(stderr, "Error: Memory allocation failed for SymbolTable.\n");
        exit(1);
    }
    table->current_scope = create_scope(nullptr); // global scope has no parent
    table->level = 0;
    return table;
}

// Destroy all symbols in a hash table
static void destroy_hash_table(Symbol* hash_table[]) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        Symbol* symbol = hash_table[i];
        while (symbol) {
            Symbol* temp = symbol;
            symbol = symbol->next;
            free(temp->name);
            free(temp);
        }
    }
}

// Destroy a scope and all its symbols
static void destroy_scope(Scope* scope) {
    if (scope) {
        destroy_hash_table(scope->hash_table);
        free(scope);
    }
}

// Destroy the entire symbol table and all scopes
void destroy_symbol_table(SymbolTable* table) {
    if (table) {
        while (table->current_scope) {
            Scope* temp = table->current_scope;
            table->current_scope = table->current_scope->parent;
            destroy_scope(temp);
        }
        free(table);
    }
}

// Enter a new scope
void enter_scope(SymbolTable* table) {
    if (!table) return;
    Scope* new_scope = create_scope(table->current_scope);
    new_scope->parent = table->current_scope;
    table->current_scope = new_scope;
    table->level++;
}

// Exit the current scope
void exit_scope(SymbolTable* table) {
    if (!table || !table->current_scope) return;
    if (table->current_scope->parent == NULL) {
        fprintf(stderr, "Error: Top level return.\n");
        return;
    }
    Scope* temp = table->current_scope;
    table->current_scope = table->current_scope->parent;
    table->level--;
    destroy_scope(temp);
}

// Add a symbol to the current scope
int64_t add_symbol(SymbolTable* table, const char* name, SymbolType type) {
    if (!table || !name) return -1;
    unsigned long index = hash(name);
    Scope* scope = table->current_scope;

    // Check for duplicate in current scope
    Symbol* existing = scope->hash_table[index];
    while (existing) {
        if (strcmp(existing->name, name) == 0) {
            fprintf(stderr, "Error: Duplicate symbol '%s' in the current scope.\n", name);
            return -1;
        }
        existing = existing->next;
    }

    Symbol* new_symbol = (Symbol*)malloc(sizeof(Symbol));
    if (!new_symbol) {
        fprintf(stderr, "Error: Memory allocation failed for Symbol.\n");
        return -1;
    }
    new_symbol->name = strdup(name);
    new_symbol->type = type;
    new_symbol->next = nullptr;

    if (type == SYMBOL_VARIABLE) {
        new_symbol->data.variable.is_initialized = false;
        new_symbol->data.variable.index = scope->variable_index_counter++;
    }

    // Chaining
    new_symbol->next = scope->hash_table[index];
    scope->hash_table[index] = new_symbol;

    return index;
}

int64_t add_function_symbol(SymbolTable* table, const char* name, size_t arity) {
    if (!table || !name) return -1;
    unsigned long index = hash(name);
    Scope* scope = table->current_scope;

    // Check for duplicate in current scope
    Symbol* existing = scope->hash_table[index];
    while (existing) {
        if (strcmp(existing->name, name) == 0) {
            fprintf(stderr, "Error: function '%s' already defined.\n", name);
            return -1;
        }
        existing = existing->next;
    }

    Symbol* new_symbol = (Symbol*)malloc(sizeof(Symbol));
    if (!new_symbol) {
        fprintf(stderr, "Error: Memory allocation failed for function.\n");
        return -1;
    }
    new_symbol->name = strdup(name);
    new_symbol->type = SYMBOL_FUNCTION;
    new_symbol->data.function.arity = arity;
    new_symbol->next =  scope->hash_table[index];
    scope->hash_table[index] = new_symbol;

    return index;
}

// Lookup a symbol by name (searches from current scope upwards)
Symbol* lookup_symbol(SymbolTable* table, const char* name) {
    if (!table || !name) return nullptr;

    unsigned long index = hash(name);
    Scope* scope = table->current_scope;

    while (scope) {
        Symbol* symbol = scope->hash_table[index];
        while (symbol) {
            if (strcmp(symbol->name, name) == 0) {
                return symbol;
            }
            symbol = symbol->next;
        }
        scope = scope->parent;
    }

    // Symbol not found
    return nullptr;
}