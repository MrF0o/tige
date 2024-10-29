//
// Created by fathi on 10/20/2024.
//

#ifndef TIGE_ERROR_H
#define TIGE_ERROR_H

#include <stddef.h>

typedef enum {
    ERROR_LEXER,
    ERROR_PARSER,
    ERROR_RUNTIME,
    // Add more error types if needed
} ErrorType;

typedef struct {
    ErrorType type;
    int line;
    int column;
    char* message;
} Error;

typedef struct {
    Error** errors;
    size_t count;
    size_t capacity;
} ErrorList;

// Function prototypes
ErrorList* create_error_list();
void add_error(ErrorList* list, Error* error);
void free_error_list(ErrorList* list);
void print_errors(const ErrorList* list);
bool is_error_list_empty(const ErrorList* list);
#endif //TIGE_ERROR_H
