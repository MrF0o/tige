//
// Created by fathi on 10/19/2024.
//

#ifndef TIGE_PARSER_H
#define TIGE_PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct Parser Parser;
typedef struct Context Context;

#define MATCH(parser, ...) matches(parser, __VA_ARGS__, -1)
#define CURRENT(parser, ...) matches_current(parser, __VA_ARGS__, -1)

struct Parser {
    Context* context;
    Token *current_token;
    Lexer *lexer;
    TokenList *token_list;
    unsigned int current_token_index;
};

void parser_init(Parser *parser, Context* ctx);
bool parser_is_initialized(Parser *parser);
void advance(Parser *parser);

Token *peek(Parser *parser);

bool expect(Parser *parser, TokenType tokenType);

unsigned char matches(Parser *parser, ...);

bool matches_current(Parser *parser, ...);

ASTNode *parse(Context *ctx);

/* STATEMENTS */
ASTNode *parse_program(Parser *parser);

ASTNode *parse_decl_stmt(Parser *parser);

ASTNode* parse_fn_decl_stmt(Parser* parser);

ASTNode *parse_var_decl_stmt(Parser *parser);

ASTNode *parse_statement(Parser *parser);

ASTNode *parse_expression_stmt(Parser *parser);

ASTNode *parse_block_stmt(Parser *parser);

ASTNode *parse_if_stmt(Parser *parser);

ASTNode *parse_loop_stmt(Parser *parser);

ASTNode *parse_for_stmt(Parser *parser);

ASTNode *parse_break_stmt(Parser *parser);

ASTNode *parse_return_stmt(Parser *parser);

/* EXPRESSIONS */

ASTNode* parse_range_expr(Parser* parser);

ASTNode *parse_expression(Parser *parser);

ASTNode *parse_assignment_expression(Parser *parser);

ASTNode *parse_ternary_expression(Parser *parser);

ASTNode *parse_logical_or_expression(Parser *parser);

ASTNode *parse_logical_and_expression(Parser *parser);

ASTNode *parse_equality_expression(Parser *parser);

ASTNode *parse_relational_expression(Parser *parser);

ASTNode *parse_additive_expression(Parser *parser);

ASTNode *parse_multiplicative_expression(Parser *parser);

ASTNode *parse_unary_expression(Parser *parser);

ASTNode *parse_primary_expression(Parser *parser);

ASTNode *parse_binary_expression(Parser *parser);

ASTNode *parse_literal(Parser *parser);

ASTNode *parse_number(Parser *parser);

ASTNode *parse_string(Parser *parser);

// utilities
unsigned char is_parse_end(Parser *parser);


#endif //TIGE_PARSER_H
