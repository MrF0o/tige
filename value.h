//
// Created by fathi on 10/20/2024.
//

#ifndef TIGE_VALUE_H
#define TIGE_VALUE_H

#include <stdint.h>
#include <stdbool.h>

#define IS_NULL(val) (((val)->type == VAL_PTR) && ((val)->as_ptr == nullptr))

typedef enum {
    VAL_INT,
    VAL_FLOAT,
    VAL_BOOL,
    VAL_STRING,
    VAL_OBJECT,
    VAL_PTR,
    VAL_NULL,
} ValueType;

typedef struct {
    ValueType type;
    union {
        int64_t as_integer;
        double as_float;
        bool as_boolean;
        char* as_string;
        uint64_t as_ptr;
    };
} Value;

// primitives
Value make_int(int64_t x);
Value make_float(double x);
Value make_bool(bool x);
Value make_string(const char* x);
Value make_null();

void print_value(Value value);

#endif //TIGE_VALUE_H
