//
// Created by fathi on 10/20/2024.
//

#ifndef TIGE_SYMBOL_H
#define TIGE_SYMBOL_H
#include <stddef.h>
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
            uint16_t index;
        } variable;

        // For functions
        struct {
            size_t arity;
            uint16_t arg_b;
            uint16_t arg_e;
        } function;
    } data;

    struct Symbol* next;
} Symbol;
#endif //TIGE_SYMBOL_H
