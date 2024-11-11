//
// Created by fathi on 10/19/2024.
//

#include <string.h>
#include <stdio.h>
#include "stdlib.h"
#include "ast.h"

#include "tige_string.h"

void free_ast_node(ASTNode* node);

ASTNode* create_ast(ASTNodeType type, ASTValue* value)
{
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));

    if (node)
    {
        node->type = type;
        node->value = value;
    }

    return node;
}

ASTValue* create_float_value(double val)
{
    ASTValue* value = (ASTValue*)malloc(sizeof(ASTValue));
    value->float_value = val;
    return value;
}

ASTValue* create_int_value(long long val)
{
    ASTValue* value = (ASTValue*)malloc(sizeof(ASTValue));
    value->int_value = val;
    return value;
}

ASTValue* create_string_value(const char* str)
{
    TString* tstr = new_string();
    if (tstr)
    {
        tstr->chars = strdup(str);
        const auto value = (ASTValue*)malloc(sizeof(ASTValue));
        value->str_value = tstr;
        return value;
    }
    return nullptr;
}

ASTValue* create_bool_value(bool val)
{
    ASTValue* value = (ASTValue*)malloc(sizeof(ASTValue));
    if (value)
    {
        value->bool_value = val;
    }
    return value;
}

ASTNodeList* create_ast_node_list()
{
    ASTNodeList* list = malloc(sizeof(ASTNodeList));
    list->capacity = 8;
    list->count = 0;
    list->nodes = malloc(sizeof(ASTNode*) * list->capacity);
    return list;
}

void ast_node_list_add(ASTNodeList* list, ASTNode* node)
{
    if (list->count >= list->capacity)
    {
        list->capacity *= 2;
        list->nodes = realloc(list->nodes, sizeof(ASTNode*) * list->capacity);
    }
    list->nodes[list->count++] = node;
}

void free_ast_node_list(ASTNodeList* list)
{
    for (size_t i = 0; i < list->count; i++)
    {
        free_ast_node(list->nodes[i]); // Implement free_ast_node recursively
    }
    free(list->nodes);
    free(list);
}

void free_ast_node(ASTNode* node)
{
    if (node == NULL)
    {
        return;
    }

    switch (node->type)
    {
    case AST_INTEGER:
    case AST_FLOAT:
    case AST_BOOL:
        if (node->value != NULL)
        {
            free(node->value);
        }
        break;

    case AST_STRING:
    case AST_SYMBOL:
        if (node->value != NULL)
        {
            if (node->value->str_value != NULL)
            {
                free(node->value->str_value);
            }
            free(node->value);
        }
        break;

    case AST_BINARY_OP:
        free_ast_node(node->binary_op_expr.left);
        free_ast_node(node->binary_op_expr.right);
        break;

    case AST_UNARY_OP:
        free_ast_node(node->unary_op_expr.operand);
        break;

    case AST_COMPARE:
        free_ast_node(node->compare_expr.left);
        free_ast_node(node->compare_expr.right);
        break;

    case AST_ASSIGN:
        free_ast_node(node->assignment_expr.left);
        free_ast_node(node->assignment_expr.right);
        break;

    case AST_TERNARY_OP:
        free_ast_node(node->ternary_op_expr.condition);
        free_ast_node(node->ternary_op_expr.true_expr);
        free_ast_node(node->ternary_op_expr.false_expr);
        break;

    case AST_EXPRESSION_STMT:
        free_ast_node(node->expression_stmt.expression);
        break;

    case AST_BLOCK:
        if (node->block.statements != NULL)
        {
            for (size_t i = 0; i < node->block.statement_count; i++)
            {
                free_ast_node(node->block.statements[i]);
            }
        }
        break;

    case AST_IF:
        free_ast_node(node->if_stmt.condition);
        free_ast_node(node->if_stmt.then_branch);
        if (node->if_stmt.else_branch != NULL)
        {
            free_ast_node(node->if_stmt.else_branch);
        }
        break;

    case AST_LOOP:
        free_ast_node(node->loop_stmt.body);
        break;

    case AST_FOR:
        if (node->for_stmt.identifier != NULL)
        {
            free(node->for_stmt.identifier);
        }
        free_ast_node(node->for_stmt.range);
        free_ast_node(node->for_stmt.body);
        break;

    case AST_BREAK:
        // No dynamically allocated members to free
        break;

    case AST_RETURN:
        if (node->return_stmt.value != NULL)
        {
            free_ast_node(node->return_stmt.value);
        }
        break;

    case AST_VAR_DECL:
        if (node->var_decl_expr.identifier != NULL)
        {
            free(node->var_decl_expr.identifier);
        }
        if (node->var_decl_expr.value != NULL)
        {
            free_ast_node(node->var_decl_expr.value);
        }
        break;

    case AST_CALL:

        if (node->call_expr.arguments != NULL)
        {
            free_ast_node_list(node->call_expr.arguments);
        }
        break;

    case AST_FN_DECL:
        free_ast_node(node->fn_decl_stmt.body);
        free_ast_node_list(node->fn_decl_stmt.params);
        free(node->fn_decl_stmt.identifier);
        break;

    case AST_RANGE:
        free_ast_node(node->range_expr.start);
        free_ast_node(node->range_expr.end);
        break;

    default:
        // TODO
        break;
    }

    // Finally, free the AST node itself
    free(node);
}

ASTNode* create_for_stmt(const char* identifier, ASTNode* range, ASTNode* body)
{
    ASTNode* node = create_ast(AST_FOR, NULL);
    node->for_stmt.identifier = strdup(identifier);
    node->for_stmt.range = range;
    node->for_stmt.body = body;
    return node;
}


ASTNode* create_range_expr(ASTNode* start, ASTNode* end)
{
    ASTNode* node = create_ast(AST_RANGE, NULL);
    node->range_expr.start = start;
    node->range_expr.end = end;
    return node;
}

ASTNode* create_block(ASTNode** statements, size_t count)
{
    ASTNode* node = create_ast(AST_BLOCK, NULL);
    node->block.statements = malloc(sizeof(ASTNode*) * count);
    if (!node->block.statements)
    {
        fprintf(stderr, "Memory allocation failed for block statements.\n");
        exit(1);
    }
    memcpy(node->block.statements, statements, sizeof(ASTNode*) * count);
    node->block.statement_count = count;
    return node;
}


ASTNode* create_if_stmt(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch)
{
    ASTNode* node = create_ast(AST_IF, NULL);
    node->if_stmt.condition = condition;
    node->if_stmt.then_branch = then_branch;
    node->if_stmt.else_branch = else_branch;
    return node;
}

ASTNode* create_fn_decl_stmt(char* name, ASTNodeList* params, ASTNode* body)
{
    ASTNode* node = create_ast(AST_FN_DECL, nullptr);
    node->fn_decl_stmt.identifier = name;
    node->fn_decl_stmt.params = params;
    node->fn_decl_stmt.body = body;
    return node;
}

ASTNode* create_expression_stmt(ASTNode* expression)
{
    ASTNode* node = create_ast(AST_EXPRESSION_STMT, nullptr);
    node->expression_stmt.expression = expression;

    return node;
}
