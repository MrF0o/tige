//
// Created by fathi on 10/20/2024.
//

#ifndef TIGE_SYMBOL_H
#define TIGE_SYMBOL_H
#include <stdbool.h>
#include "value.h"

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    // Future extensions: SYMBOL_CLASS, SYMBOL_TRAIT, etc.
} SymbolType;

typedef struct Symbol {
    char* name;
    SymbolType type;

    union {
        struct {
            char* var_type;
            bool is_initialized;
            Value value;
            uint16_t index;
        } variable;

        // For functions
        struct {
            char* return_type;
            // Parameters, etc.
            // For simplicity, we can add a parameter list later
        } function;

        // Future extensions for classes, traits, etc.
    } data;

    struct Symbol* next; // For handling collisions in the hash table
} Symbol;
#endif //TIGE_SYMBOL_H
