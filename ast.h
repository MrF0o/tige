//
// Created by fathi on 10/19/2024.
//

#ifndef TIGE_AST_H
#define TIGE_AST_H

#include "lexer.h"
#include "object.h"


typedef enum {
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_COMPARE,
    AST_ASSIGN,
    AST_TERNARY_OP,
    AST_INTEGER,
    AST_FLOAT,
    AST_SYMBOL,
    AST_BOOL,
    AST_STRING,
    AST_CALL,
    AST_EXPRESSION_STMT,
    AST_VAR_DECL,
    AST_FN_DECL,
    AST_BLOCK,
    AST_IF,
    AST_LOOP,
    AST_FOR,
    AST_BREAK,
    AST_RETURN,
    AST_RANGE,
} ASTNodeType;

typedef union {
    TString *str_value;        // For strings, identifiers
    int64_t int_value;    // For integers
    double float_value;     // For floats
    bool bool_value;        // For booleans
    void *value;            // For other types, especially pointers
} ASTValue;

typedef struct ASTNode ASTNode;

typedef struct ASTNodeList {
    ASTNode **nodes;
    size_t count;
    size_t capacity;
} ASTNodeList;

struct ASTNode {
    ASTNodeType type;

    union {
        ASTValue *value;
        TObject* object;

        // Binary operation
        struct {
            ASTNode *left;
            TokenType operator;
            ASTNode *right;
        } binary_op_expr;

        // Unary operation
        struct {
            ASTNode *operand;
            TokenType operator;
        } unary_op_expr;

        // Comparison expression
        struct {
            ASTNode *left;
            TokenType operator;
            ASTNode *right;
        } compare_expr;

        // Assignment expression
        struct {
            ASTNode *left;
            TokenType operator;
            ASTNode *right;
        } assignment_expr;

        // Ternary operation
        struct {
            ASTNode *condition;
            ASTNode *true_expr;
            ASTNode *false_expr;
        } ternary_op_expr;

        // Call expressions
        struct {
            ASTNode *callee;
            ASTNodeList *arguments;
        } call_expr;

        // Expression Statement
        struct {
            ASTNode *expression;
        } expression_stmt;

        // Variable declaration
        struct {
            char* identifier;
            ASTNode* value;
        } var_decl_expr;

        // Block
        struct {
            ASTNode **statements;
            size_t statement_count;
        } block;

        // If Statement
        struct {
            ASTNode *condition;
            ASTNode *then_branch;
            ASTNode *else_branch; // Optional
        } if_stmt;

        // Loop Statement
        struct {
            ASTNode *body;
        } loop_stmt;

        struct {
            ASTNode *start;
            ASTNode *end;
        } range_expr;

        struct {
            char *identifier;
            ASTNode *range;
            ASTNode *body;
        } for_stmt;

        // Break Statement
        struct {
            // no additional data
        } break_stmt;

        struct {
            char* identifier; // none if the function is a closure
            ASTNodeList* params;
            ASTNode* body;
        } fn_decl_stmt;

        // Return Statement
        struct {
            ASTNode *value; // Could be NULL
        } return_stmt;
    };
};

// Function prototypes
ASTNode *create_ast(ASTNodeType type, ASTValue *value);

void free_ast_node(ASTNode *node);

ASTValue *create_float_value(double value);

ASTValue *create_int_value(long long val);

ASTValue *create_string_value(const char *str);

ASTValue *create_bool_value(bool val);

ASTNodeList *create_ast_node_list();

void ast_node_list_add(ASTNodeList *list, ASTNode *node);

void free_ast_node_list(ASTNodeList *list);

ASTNode *create_for_stmt(const char *identifier, ASTNode *range, ASTNode *body);

ASTNode *create_range_expr(ASTNode *start, ASTNode *end);

ASTNode *create_block(ASTNode **statements, size_t count);

ASTNode* create_if_stmt(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch);

ASTNode* create_expression_stmt(ASTNode* expression);

ASTNode* create_fn_decl_stmt(char* name, ASTNodeList* params, ASTNode* body);

#endif //TIGE_AST_H