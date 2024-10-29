//
// Created by fathi on 10/21/2024.
//

#ifndef TIGE_COMPILER_H
#define TIGE_COMPILER_H

#include "ast.h"
#include "bytecode_buffer.h"

typedef struct Context Context;

// Function to compile an AST into bytecode
BytecodeBuffer* compile_ast(ASTNode *node, Context* ctx);

void compile_integer(BytecodeBuffer* buffer, ASTNode* node);
void compile_float(BytecodeBuffer* buffer, ASTNode* node);
void compile_bool(BytecodeBuffer* buffer, ASTNode* node);
void compile_string(BytecodeBuffer* buffer, ASTNode* node);
void compile_symbol(BytecodeBuffer* buffer, ASTNode* node);
void compile_binary_op(BytecodeBuffer* buffer, ASTNode* node);
void compile_unary_op(BytecodeBuffer* buffer, ASTNode* node);
void compile_ternary_op(BytecodeBuffer* buffer, ASTNode* node);
void compile_compare(BytecodeBuffer* buffer, ASTNode* node);
void compile_assign(BytecodeBuffer* buffer, ASTNode* node);
void compile_call(BytecodeBuffer* buffer, ASTNode* node);
void compile_expression_statement(BytecodeBuffer* buffer, ASTNode* node);
void compile_block(BytecodeBuffer* buffer, ASTNode* node);
void compile_if(BytecodeBuffer* buffer, ASTNode* node);
void compile_loop(BytecodeBuffer* buffer, ASTNode* node);
void compile_for(BytecodeBuffer* buffer, ASTNode* node);
void compile_break(BytecodeBuffer* buffer, ASTNode* node);
void compile_return(BytecodeBuffer* buffer, ASTNode* node);
void compile_range(BytecodeBuffer* buffer, ASTNode* node);
void compile_var_decl(BytecodeBuffer* buffer, ASTNode* node);
void compile_fn_decl(BytecodeBuffer *buffer, ASTNode *node);

#endif //TIGE_COMPILER_H
