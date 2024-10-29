//
// Created by fathi on 10/21/2024.
//

#ifndef TIGE_AST_UTILS_H
#define TIGE_AST_UTILS_H
#include "ast.h"

#define AST_IS_BINARY_OP(node)    ((node)->type == AST_BINARY_OP)
#define AST_IS_UNARY_OP(node)     ((node)->type == AST_UNARY_OP)
#define AST_IS_COMPARE(node)      ((node)->type == AST_COMPARE)
#define AST_IS_ASSIGN(node)       ((node)->type == AST_ASSIGN)
#define AST_IS_TERNARY_OP(node)   ((node)->type == AST_TERNARY_OP)
#define AST_IS_INTEGER(node)      ((node)->type == AST_INTEGER)
#define AST_IS_FLOAT(node)        ((node)->type == AST_FLOAT)
#define AST_IS_SYMBOL(node)       ((node)->type == AST_SYMBOL)
#define AST_IS_BOOL(node)         ((node)->type == AST_BOOL)
#define AST_IS_STRING(node)       ((node)->type == AST_STRING)
#define AST_IS_CALL(node)         ((node)->type == AST_CALL)
#define AST_IS_EXPRESSION_STMT(node) ((node)->type == AST_EXPRESSION_STMT)
#define AST_IS_BLOCK(node)        ((node)->type == AST_BLOCK)
#define AST_IS_IF(node)           ((node)->type == AST_IF)
#define AST_IS_LOOP(node)         ((node)->type == AST_LOOP)
#define AST_IS_FOR(node)          ((node)->type == AST_FOR)
#define AST_IS_BREAK(node)        ((node)->type == AST_BREAK)
#define AST_IS_RETURN(node)       ((node)->type == AST_RETURN)
#define AST_IS_RANGE(node)        ((node)->type == AST_RANGE)
#define AST_AS_BINARY_OP(node)       (&(node)->binary_op_expr)
#define AST_AS_UNARY_OP(node)        (&(node)->unary_op_expr)
#define AST_AS_COMPARE(node)         (&(node)->compare_expr)
#define AST_AS_ASSIGN(node)          (&(node)->assignment_expr)
#define AST_AS_TERNARY_OP(node)      (&(node)->ternary_op_expr)
#define AST_AS_CALL(node)            (&(node)->call_expr)
#define AST_AS_EXPRESSION_STMT(node) ((node)->expression_stmt.expression)
#define AST_AS_BLOCK(node)           ((node)->block.statements)
#define AST_AS_IF(node)              (&(node)->if_stmt)
#define AST_AS_FOR(node)             (&(node)->for_stmt)
#define AST_AS_RETURN(node)          (&(node)->return_stmt)
#define AST_AS_RANGE(node)           (&(node)->range_expr)
#endif //TIGE_AST_UTILS_H
