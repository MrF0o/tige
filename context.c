//
// Created by fathi on 10/20/2024.
//

#include "context.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "error.h"
#include "memory.h"
#include "compiler.h"
#include "uthash.h"

void ctx_init(Context *ctx, const char *source_code) {
    ctx->source_length = strlen(source_code);
    ctx->source = (char *) tige_alloc(ctx, ctx->source_length + 1);
    strcpy(ctx->source, source_code);

    ctx->error_list = create_error_list();
    lexer_init(&ctx->lexer, ctx->source);
    parser_init(&ctx->parser, ctx);

    // as soon as we encounter a block, this will be initialized
    ctx->symbols = nullptr;
    ctx->vm = nullptr;
}

void ctx_free(Context *ctx) {
    if (ctx->source != nullptr) {
        tige_free(ctx, ctx->source);
        ctx->source = nullptr;
    }

    if (ctx->error_list != nullptr) {
        free_error_list(ctx->error_list);
        ctx->error_list = nullptr;
    }

    destroy_symbol_table(ctx->symbols);
    bc_destroy_bytecode_buffer(ctx->code);
}

void ctx_destroy(Context* context) {
    destroy_vm(context->vm);
    ctx_free(context);
}

// TODO: multithreading
void vm_swap_code_buffer(VM *vm, BytecodeBuffer *buffer) {
    // sometimes we need to switch the buffer in the middle of another so
    // TODO: save the last buffer execution state for later resume
    if (bc_is_buffer_valid(buffer)) {
        vm->bytecode = buffer->current_chunk->bytecode;
        vm->size = bc_get_total_bytecode_size(buffer);
        vm->buffer = buffer;
        vm->chunk = buffer->current_chunk;
    }
}

void ctx_get_compiled_code(Context *context, BytecodeBuffer **buf) {
    if (buf) {
        if (!(*buf = context->code)) {
            *buf = bc_buffer_create();
        }
    }
}

VM *ctx_get_active_vm(Context *context) {
    // there can be multiple VMs at once
    return context->vm;
}

void ctx_create_vm(Context *context) {
    context->vm = create_vm(context);
}

bool ctx_is_vm_initialized(Context *context) {
    return context->vm != nullptr;
}

void ctx_clean_parse_info(Context *context) {
    free_ast_node(context->ast);
    context->ast = nullptr;
}

bool ctx_check_errors(Context *context) {
    return !is_error_list_empty(context->error_list);
}

void ctx_start_parsing(Context *context) {
    auto ast = parse(context);
    if (ast) {
        context->ast = ast;
        context->vm = create_vm(context);
        context->code = compile_ast(ast, context);
    }
}

bool ctx_is_initialized(Context *context) {
    return parser_is_initialized(&context->parser) &&
           lexer_is_initialized(&context->lexer)
           && context->source != nullptr;
}

bool register_function(Context *context, const char *name, Function *function_obj) {
    if (!context || !name || !function_obj) {
        fprintf(stderr, "Invalid arguments to register_function.\n");
        return false;
    }

    // Allocate memory for the FunctionEntry
    FunctionEntry *entry = (FunctionEntry *)vm_malloc(sizeof(FunctionEntry));
    if (!entry) {
        fprintf(stderr, "Failed to allocate memory for FunctionEntry.\n");
        return false;
    }

    // Duplicate the function name string
    entry->name = strdup(name);
    if (!entry->name) {
        fprintf(stderr, "Failed to allocate memory for function name.\n");
        vm_free(entry);
        return false;
    }

    entry->function = function_obj;

    // Add the entry to the hash map
    HASH_ADD_KEYPTR(hh, context->functions, entry->name, strlen(entry->name), entry);

    return true;
}

Function *get_function(Context *context, const char *name) {
    if (!context || !name) {
        fprintf(stderr, "Invalid arguments to get_function.\n");
        return nullptr;
    }

    FunctionEntry *entry = nullptr;
    HASH_FIND_STR(context->functions, name, entry);

    if (entry) {
        return entry->function;
    } else {
        fprintf(stderr, "Function '%s' not found in the context.\n", name);
        return nullptr;
    }
}

bool remove_function(Context *context, const char *name) {
    if (!context || !name) {
        fprintf(stderr, "Invalid arguments to remove_function.\n");
        return false;
    }

    FunctionEntry *entry = NULL;
    HASH_FIND_STR(context->functions, name, entry);

    if (entry) {
        HASH_DEL(context->functions, entry);
        free(entry->name);
        vm_free(entry);
        return true;
    } else {
        fprintf(stderr, "Function '%s' not found. Cannot remove.\n", name);
        return false;
    }
}

// Destroy the entire function map
void destroy_function_map(Context *context) {
    if (!context) return;

    FunctionEntry *current_entry, *tmp;
    HASH_ITER(hh, context->functions, current_entry, tmp) {
        HASH_DEL(context->functions, current_entry);
        free(current_entry->name);
        destroy_function(current_entry->function);
        vm_free(current_entry);
    }
}
