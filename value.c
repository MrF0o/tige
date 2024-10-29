//
// Created by fathi on 10/23/2024.
//

#include <stdio.h>
#include "value.h"

Value make_int(int64_t x) {
    Value value;
    value.type = VAL_INT;
    value.as_integer = x;
    return value;
}

Value make_bool(bool x) {
    Value value;
    value.type = VAL_BOOL;
    value.as_boolean = x;
    return value;
}

Value make_float(double x) {
    Value value;
    value.type = VAL_FLOAT;
    value.as_float = x;
    return value;
}

Value make_null() {
    Value value;
    value.type = VAL_FLOAT;
    value.as_ptr = 0x00;
    return value;
}

Value make_string(char *x) {
    return make_null();
}

void print_value(Value value) {
    switch (value.type) {
        case VAL_INT:
            printf("INT(%lld)", value.as_integer);
            break;
        case VAL_BOOL:
            printf("BOOL(%s)", value.as_boolean ? "true" : "false");
            break;
        case VAL_STRING:
            printf("STRING(\"%s\")", value.as_string);
            break;
        case VAL_FLOAT:
            printf("FLOAT(%lf)", value.as_float);
            break;
        default:
            printf("UNKNOWN");
    }
    printf("\n");
}