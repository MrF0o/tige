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
    auto body = ast->block.statements[1]->for_stmt.body;
    if (ast) {
        context->ast = ast;
        context->code = compile_ast(ast, context);
    }
}

bool ctx_is_initialized(Context *context) {
    return parser_is_initialized(&context->parser) &&
           lexer_is_initialized(&context->lexer)
           && context->source != nullptr;
}