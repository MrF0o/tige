#include "compiler.h"
#include "symbol_table.h"
#include "context.h"
#include "ast_utils.h"
#include "memory.h"
#include "vm.h"
#include "functions.h"
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

typedef uint16_t Reg;

static Context *gcontext;

void compile_node(ASTNode *node, BytecodeBuffer *buffer) {
    switch (node->type) {
        case AST_INTEGER:
            compile_integer(buffer, node);
            break;
        case AST_FLOAT:
            compile_float(buffer, node);
            break;
        case AST_BOOL:
            compile_bool(buffer, node);
            break;
        case AST_STRING:
            compile_string(buffer, node);
            break;
        case AST_SYMBOL:
            compile_symbol(buffer, node);
            break;
        case AST_BINARY_OP:
            compile_binary_op(buffer, node);
            break;
        case AST_UNARY_OP:
            compile_unary_op(buffer, node);
            break;
        case AST_TERNARY_OP:
            compile_ternary_op(buffer, node);
            break;
        case AST_COMPARE:
            compile_compare(buffer, node);
            break;
        case AST_ASSIGN:
            compile_assign(buffer, node);
            break;
        case AST_CALL:
            compile_call(buffer, node);
            break;
        case AST_EXPRESSION_STMT:
            compile_expression_statement(buffer, node);
            break;
        case AST_BLOCK:
            compile_block(buffer, node);
            break;
        case AST_IF:
            compile_if(buffer, node);
            break;
        case AST_LOOP:
            compile_loop(buffer, node);
            break;
        case AST_FOR:
            compile_for(buffer, node);
            break;
        case AST_BREAK:
            compile_break(buffer, node);
            break;
        case AST_RETURN:
            compile_return(buffer, node);
            break;
        case AST_RANGE:
            compile_range(buffer, node);
            break;
        case AST_VAR_DECL:
            compile_var_decl(buffer, node);
            break;
        case AST_FN_DECL:
            compile_fn_decl(buffer, node);
            break;
        default:
            fprintf(stderr, "Unknown AST node type: 0x%02x\n", node->type);
            exit(1);
    }
}

void compile_fn_decl(BytecodeBuffer *buffer, ASTNode *node) {

    char *func_name = node->fn_decl_stmt.identifier;
    auto arg_list = node->fn_decl_stmt.params;
    auto argc = node->fn_decl_stmt.params->count;
    add_function_symbol(gcontext->symbols, func_name, argc);
    auto fn_sym = lookup_symbol(gcontext->symbols, func_name);

    enter_scope(gcontext->symbols);

    fn_sym->data.function.arg_b = gcontext->symbols->current_scope->variable_index_counter;
    // define all params
    for (size_t i = 0; i < argc; ++i) {
        auto arg_name = arg_list->nodes[i]->value->str_value;
        add_symbol(gcontext->symbols, arg_name, SYMBOL_VARIABLE);
    }
    fn_sym->data.function.arg_e = gcontext->symbols->current_scope->variable_index_counter;

    // do not link function chunk since it's only accessible by calling/jumping to it
    bc_start_non_linked_chunk(buffer);
    compile_node(node->fn_decl_stmt.body, buffer);
    auto chunk = bc_end_non_linked_chunk(buffer);
    bc_end_non_linked_chunk(buffer);

    Function* function = create_function();
    function->props = nullptr;
    function->metadata = nullptr;
    function->stack = get_vm()->stack;
    function->arity = argc;
    function->return_addr.chunk = buffer->current_chunk;
    function->return_addr.offset = -1;
    function->chunk = chunk;
    function->name = strdup(func_name);

    register_function(gcontext, func_name, function);

    exit_scope(gcontext->symbols);
}

// Compile the entire AST
BytecodeBuffer *compile_ast(ASTNode *node, Context *context) {
    gcontext = context;
    BytecodeBuffer *buffer = bc_buffer_create();

    // Compile the AST nodes
    compile_node(node, buffer);

    bc_emit_opcode(buffer, OP_HALT);
    return buffer;
}

////////////////////////////////////////////////////////////////////////////////
// Compiler implementation
////////////////////////////////////////////////////////////////////////////////

/// Compile Integer AST Node
void compile_integer(BytecodeBuffer *buffer, ASTNode *node) {
    bc_emit_opcode_with_int(buffer, OP_LOAD_CONST_INT, node->value->int_value);
}

/// Compile Float AST Node
void compile_float(BytecodeBuffer *buffer, ASTNode *node) {
    // TODO: Implement proper float handling
    bc_emit_opcode_with_float(buffer, OP_LOAD_CONST_FLOAT, node->value->float_value);
}

/// Compile Bool AST Node
void compile_bool(BytecodeBuffer *buffer, ASTNode *node) {
    // Emit LOAD_BOOL with 1 (true) or 0 (false)
    bc_emit_opcode_with_byte(buffer, OP_LOAD_BOOL, node->value->bool_value ? 1 : 0);
}

/// Compile String AST Node
void compile_string(BytecodeBuffer *buffer, ASTNode *node) {
    // Emit LOAD_STRING with a pointer to the string on the heap
    // Allocate the string on the heap and get its address
    // TODO: Implement string allocation and heap integration
    fprintf(stderr, "String literals not implemented in compiler.\n");
    exit(1);
}

/// Compile Symbol AST Node
void compile_symbol(BytecodeBuffer *buffer, ASTNode *node) {
    Symbol *sym = lookup_symbol(gcontext->symbols, node->value->str_value);

    if (sym) {
        if (sym->type == SYMBOL_VARIABLE) {
            bc_emit_opcode_with_uint16(buffer, OP_LOAD_VAR, sym->data.variable.index);
        } else {
            fprintf(stderr, "Error: '%s' is not a variable\n", node->value->str_value);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Error: '%s' is not defined\n", node->value->str_value);
        exit(EXIT_FAILURE);
    }
}

/// Compile Binary Operation AST Node
void compile_binary_op(BytecodeBuffer *buffer, ASTNode *node) {
    // Compile left and right operands
    compile_node(node->binary_op_expr.left, buffer);
    compile_node(node->binary_op_expr.right, buffer);

    // Handle operator
    switch (node->binary_op_expr.operator) {
        case TOKEN_PLUS:
            bc_emit_opcode(buffer, OP_ADD);
            break;
        case TOKEN_MINUS:
            bc_emit_opcode(buffer, OP_SUB);
            break;
        case TOKEN_ASTERISK:
            bc_emit_opcode(buffer, OP_MUL);
            break;
        case TOKEN_SLASH:
            bc_emit_opcode(buffer, OP_DIV);
            break;
        case TOKEN_EQ:
            bc_emit_opcode(buffer, OP_EQUAL);
            break;
        case TOKEN_NEQ:
            bc_emit_opcode(buffer, OP_NOT_EQUAL);
            break;
        case TOKEN_LT:
            bc_emit_opcode(buffer, OP_LESS_THAN);
            break;
        case TOKEN_GT:
            bc_emit_opcode(buffer, OP_GREATER_THAN);
            break;
        case TOKEN_LTE:
            bc_emit_opcode(buffer, OP_LESS_EQUAL);
            break;
        case TOKEN_GTE:
            bc_emit_opcode(buffer, OP_GREATER_EQUAL);
            break;
        default:
            fprintf(stderr, "Unsupported binary operator: %d\n", node->binary_op_expr.operator);
            exit(1);
    }
}

/// Compile Unary Operation AST Node
void compile_unary_op(BytecodeBuffer *buffer, ASTNode *node) {
    // Compile operand
    compile_node(node->unary_op_expr.operand, buffer);

    // Handle operator
    switch (node->unary_op_expr.operator) {
        case TOKEN_BANG:
            bc_emit_opcode(buffer, OP_NOT);
            break;
        case TOKEN_MINUS:
            bc_emit_int(buffer, 0);
            bc_emit_opcode(buffer, OP_LOAD_CONST_INT);
            bc_emit_opcode(buffer, OP_SUB);
            break;
        default:
            fprintf(stderr, "Unsupported unary operator: %c\n", node->unary_op_expr.operator);
            exit(1);
    }
}

/// Compile Ternary Operation AST Node
void compile_ternary_op(BytecodeBuffer *buffer, ASTNode *node) {
    // Compile condition
    compile_node(node->ternary_op_expr.condition, buffer);

    // Emit JMP_IF_FALSE to false_expr, with placeholder
    JumpPlaceholder jump_to_false = bc_emit_jump_with_placeholder(buffer, OP_JMP_IF_FALSE);

    // Compile true_expr
    compile_node(node->ternary_op_expr.true_expr, buffer);

    // Emit JMP to end, with placeholder
    JumpPlaceholder jump_to_end = bc_emit_jump_with_placeholder(buffer, OP_JMP);

    // Backpatch JMP_IF_FALSE to current position (start of false_expr)
    size_t else_chunk_id = buffer->current_chunk->chunk_id;
    size_t else_offset = buffer->current_chunk->size;

    bc_backpatch_jump(jump_to_false, else_chunk_id, else_offset);

    // Compile false_expr
    compile_node(node->ternary_op_expr.false_expr, buffer);

    // Backpatch JMP to end address (current position)
    size_t end_chunk_id = buffer->current_chunk->chunk_id;
    size_t end_offset = buffer->current_chunk->size;

    bc_backpatch_jump(jump_to_end, end_chunk_id, end_offset);
}

/// Compile Compare AST Node
void compile_compare(BytecodeBuffer *buffer, ASTNode *node) {
    // Compile left and right operands
    compile_node(node->compare_expr.left, buffer);
    compile_node(node->compare_expr.right, buffer);

    // Emit the comparison operator opcode
    switch (node->compare_expr.operator) {
        case TOKEN_EQ:
            bc_emit_opcode(buffer, OP_EQUAL);
            break;
        case TOKEN_NEQ:
            bc_emit_opcode(buffer, OP_NOT_EQUAL);
            break;
        case TOKEN_LT:
            bc_emit_opcode(buffer, OP_LESS_THAN);
            break;
        case TOKEN_GT:
            bc_emit_opcode(buffer, OP_GREATER_THAN);
            break;
        case TOKEN_LTE:
            bc_emit_opcode(buffer, OP_LESS_EQUAL);
            break;
        case TOKEN_GTE:
            bc_emit_opcode(buffer, OP_GREATER_EQUAL);
            break;
        default:
            fprintf(stderr, "Unsupported comparison operator: %d\n", node->compare_expr.operator);
            exit(1);
    }
}

/// Compile Assign AST Node
void compile_assign(BytecodeBuffer *buffer, ASTNode *node) {
    // Compile the right-hand side expression
    compile_node(node->assignment_expr.right, buffer);

    if (AST_IS_SYMBOL(node->assignment_expr.left)) {
        auto var_name = node->assignment_expr.left->value->str_value;
        Symbol *symbol = lookup_symbol(gcontext->symbols, var_name);

        if (symbol) {
            bc_emit_opcode_with_uint16(buffer, OP_STORE_VAR, symbol->data.variable.index);
        } else {
            fprintf(stderr, "Error: Assignment to an undeclared variable '%s'", var_name);
        }
    }
}

/// Compile Expression Statement AST Node
void compile_expression_statement(BytecodeBuffer *buffer, ASTNode *node) {
    compile_node(node->expression_stmt.expression, buffer);

    if (node->expression_stmt.expression->type != AST_RETURN
        && node->expression_stmt.expression->type != AST_ASSIGN) {
        // Optionally, pop the result if not needed
        bc_emit_opcode(buffer, OP_POP);
    }
}

/// Compile Block AST Node
void compile_block(BytecodeBuffer *buffer, ASTNode *node) {
    // since we entered a new scope we need to go down from the current scope too
    // so if this block is the root then we need to set up a global scope
    if (!gcontext->symbols) {
        gcontext->symbols = create_symbol_table();
    }

    for (size_t i = 0; i < node->block.statement_count; i++) {
        compile_node(node->block.statements[i], buffer);
    }
}

/// Compile If Statement AST Node
void compile_if(BytecodeBuffer *buffer, ASTNode *node) {
    enter_scope(gcontext->symbols);
    bc_emit_opcode(buffer, OP_SAVE_SP);

    // Compile condition
    compile_node(node->if_stmt.condition, buffer);

    // Emit JMP_IF_FALSE to else_branch, with placeholder
    JumpPlaceholder jump_to_else = bc_emit_jump_with_placeholder(buffer, OP_JMP_IF_FALSE);

    // Compile then_branch
    compile_node(node->if_stmt.then_branch, buffer);

    // Emit JMP to end, with placeholder
    JumpPlaceholder jump_to_end = bc_emit_jump_with_placeholder(buffer, OP_JMP);

    // Backpatch JMP_IF_FALSE to current position (start of else_branch or end if no else_branch)
    size_t else_chunk_id = buffer->current_chunk->chunk_id;
    size_t else_offset = buffer->current_chunk->size;

    bc_backpatch_jump(jump_to_else, else_chunk_id, else_offset);

    // Compile else_branch if it exists
    if (node->if_stmt.else_branch) {
        compile_node(node->if_stmt.else_branch, buffer);
    }

    // Backpatch JMP to end address (current position)
    size_t end_chunk_id = buffer->current_chunk->chunk_id;
    size_t end_offset = buffer->current_chunk->size;

    bc_backpatch_jump(jump_to_end, end_chunk_id, end_offset);

    bc_emit_opcode(buffer, OP_RESET_SP);
    exit_scope(gcontext->symbols);
}

/// Compile Loop AST Node
void compile_loop(BytecodeBuffer *buffer, ASTNode *node) {
    // Implement loop constructs (e.g., while loops)
    // TODO: Implement loop handling
    fprintf(stderr, "Loop statements not implemented in compiler.\n");
    exit(1);
}

/// Compile For Loop AST Node
void compile_for(BytecodeBuffer *buffer, ASTNode *node) {
    // Enter a new scope for the loop
    enter_scope(gcontext->symbols);
    // bc_emit_opcode(buffer, OP_SAVE_SP);

    // Assign a register index for the loop variable
    add_symbol(gcontext->symbols, node->for_stmt.identifier, SYMBOL_VARIABLE);
    Symbol *symbol = lookup_symbol(gcontext->symbols, node->for_stmt.identifier);

    // Assign a register index for the end value
    add_symbol(gcontext->symbols, "__end", SYMBOL_VARIABLE);
    Symbol *end_symbol = lookup_symbol(gcontext->symbols, "__end");

    // Compile the start expression
    compile_node(node->for_stmt.range->range_expr.start, buffer);
    // Store start value in the loop variable's register
    bc_emit_opcode_with_uint16(buffer, OP_STORE_VAR, symbol->data.variable.index);

    // Compile the end expression
    compile_node(node->for_stmt.range->range_expr.end, buffer);
    // Store end value in the end variable's register
    bc_emit_opcode_with_uint16(buffer, OP_STORE_VAR, end_symbol->data.variable.index);

    // Prepare labels for jumps
    size_t loop_start_chunk_id = buffer->current_chunk->chunk_id;
    size_t loop_start_offset = buffer->current_chunk->size;

    // Load loop variable and end value for comparison
    bc_emit_opcode_with_uint16(buffer, OP_LOAD_VAR, symbol->data.variable.index);
    bc_emit_opcode_with_uint16(buffer, OP_LOAD_VAR, end_symbol->data.variable.index);

    // Emit comparison (OP_LESS_THAN)
    bc_emit_opcode(buffer, OP_LESS_THAN);

    // Emit OP_JMP_IF_FALSE with a placeholder
    JumpPlaceholder exit_jump = bc_emit_jump_with_placeholder(buffer, OP_JMP_IF_FALSE);

    // Compile the loop body
    compile_node(node->for_stmt.body, buffer);

    // Increment the loop variable
    bc_emit_opcode_with_uint16(buffer, OP_INC_REG, symbol->data.variable.index);

    // Jump back to the loop start
    bc_emit_opcode_with_jump(buffer, OP_JMP, loop_start_chunk_id, loop_start_offset);

    // Backpatch the exit jump
    size_t loop_end_chunk_id = buffer->current_chunk->chunk_id;
    size_t loop_end_offset = buffer->current_chunk->size;
    bc_backpatch_jump(exit_jump, loop_end_chunk_id, loop_end_offset);

    // Exit the loop scope
    // bc_emit_opcode(buffer, OP_RESET_SP);
    exit_scope(gcontext->symbols);
}

/// Compile Break Statement AST Node
void compile_break(BytecodeBuffer *buffer, ASTNode *node) {
    // Implement break statements
    // TODO: Implement break statement handling
    fprintf(stderr, "Break statements not implemented in compiler.\n");
    exit(1);
}

/// Compile Return Statement AST Node
void compile_return(BytecodeBuffer *buffer, ASTNode *node) {
    if (node->return_stmt.value) {
        // Compile the return expression
        compile_node(node->return_stmt.value, buffer);
        // Emit RETURN opcode
        bc_emit_opcode(buffer, OP_RETURN);
    } else {
        // Emit RETURN opcode with default value (e.g., 0)
        bc_emit_opcode_with_int(buffer, OP_LOAD_CONST_INT, 0);
        bc_emit_opcode(buffer, OP_RETURN);
    }
}

/// Compile Range Expression AST Node
void compile_range(BytecodeBuffer *buffer, ASTNode *node) {
    // Implement range expressions (e.g., 1..10)
    // TODO: Implement range handling
    fprintf(stderr, "Range expressions not implemented in compiler.\n");
    exit(1);
}

/// Compile Call AST Node
void compile_call(BytecodeBuffer *buffer, ASTNode *node) {
    Symbol* fn = lookup_symbol(gcontext->symbols, node->call_expr.callee->value->str_value);

    if (!fn) {
        fprintf(stderr, "Error: Call to an undefined function '%s'", node->call_expr.callee->value->str_value);
        exit(EXIT_FAILURE);
    }

    auto argc = node->call_expr.arguments->count;
    auto arity = fn->data.function.arity;
    if (argc != arity) {
        // TODO: proper error handling
        fprintf(stderr, "Error: '%s' expects %lld argument(s) although %lld provided", fn->name, arity, argc);
        exit(EXIT_FAILURE);
    }

    // load the arguments
    for (uint16_t i = fn->data.function.arg_b; i <= fn->data.function.arg_e; ++i) {
        bc_emit_opcode_with_uint16(buffer, OP_LOAD_VAR, i);
    }

    for (size_t i = 0; i < argc; ++i) {
        compile_node(node->call_expr.arguments->nodes[i], buffer);
    }

    bc_emit_opcode_with_string(buffer, OP_CALL, fn->name);
}

/// Compile Variable Declaration AST Node
void compile_var_decl(BytecodeBuffer *buffer, ASTNode *node) {
    int64_t symbol_index = add_symbol(gcontext->symbols, node->var_decl_expr.identifier, SYMBOL_VARIABLE);
    if (symbol_index == -1) {
        fprintf(stderr, "Error: Duplicate variable '%s'.\n", node->var_decl_expr.identifier);
        return;
    }

    Symbol *symbol = lookup_symbol(gcontext->symbols, node->var_decl_expr.identifier);

    if (node->var_decl_expr.value) {
        compile_node(node->var_decl_expr.value, buffer);
    }

    bc_emit_opcode_with_uint16(buffer, OP_STORE_VAR, symbol->data.variable.index);

    // WIP...
}