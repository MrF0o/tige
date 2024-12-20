//
// Created by fathi on 10/20/2024.
//

#ifndef TIGE_CONTEXT_H
#define TIGE_CONTEXT_H

#include <stdlib.h>
#include "parser.h"
#include "lexer.h"
#include "symbol_table.h"
#include "vm.h"
#include "bytecode_buffer.h"

typedef struct Context Context;

typedef struct FunctionEntry {
    char *name;                 // Key: Function name
    Function *function;       // Value: Pointer to Function Object
    UT_hash_handle hh;          // Makes this structure hashable
} FunctionEntry;

struct Context {
    // code information
    char* source;
    size_t source_length;

    // Lexer and parser instances
    Lexer lexer;
    Parser parser;
    ASTNode* ast;

    ErrorList* error_list;

    // scope checking for the compilation phase
    SymbolTable* symbols;

    // total allocated memory
    size_t total_mem;
    // the virtual machine that will execute our bytecode
    VM* vm;
    // the compiled code from AST
    BytecodeBuffer* code;

    // Functions map
    FunctionEntry *functions;
};

bool register_function(Context *context, const char *name, Function* function_obj);
Function *get_function(Context *context, const char *name);

void ctx_init(Context* ctx, const char* source_code);
bool ctx_is_initialized(Context *context);

void ctx_start_parsing(Context *context);

bool ctx_check_errors(Context *context);

void ctx_clean_parse_info(Context *context);

bool ctx_is_vm_initialized(Context *context);

void ctx_create_vm(Context *context);

VM *ctx_get_active_vm(Context *context);

void ctx_get_compiled_code(Context *context, BytecodeBuffer **buf);

void vm_swap_code_buffer(VM *vm, BytecodeBuffer *buffer);

void ctx_destroy(Context* context);

#endif //TIGE_CONTEXT_H
