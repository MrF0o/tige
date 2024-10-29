//
// Created by fathi on 10/20/2024.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "error.h"

ErrorList* create_error_list() {
    ErrorList* list = malloc(sizeof(ErrorList));
    list->capacity = 8;
    list->count = 0;
    list->errors = malloc(sizeof(Error*) * list->capacity);
    return list;
}

void add_error(ErrorList* list, Error* error) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->errors = realloc(list->errors, sizeof(Error*) * list->capacity);
    }
    list->errors[list->count++] = error;
}

void free_error_list(ErrorList* list) {
    for (size_t i = 0; i < list->count; i++) {
        if (list->errors[i]->message != NULL) {
            free(list->errors[i]->message);
        }
        free(list->errors[i]);
    }
    free(list->errors);
    free(list);
}

void print_errors(const ErrorList* list) {
    for (size_t i = 0; i < list->count; i++) {
        Error* error = list->errors[i];
        fprintf(stderr, "[%s Error] Line %d, Column %d: %s\n",
                error->type == ERROR_LEXER ? "Lexer" : "Parser",
                error->line,
                error->column,
                error->message);
    }
}

bool is_error_list_empty(const ErrorList *list) {
    return list->count <= 0;
}
