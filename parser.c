//
// Created by fathi on 10/19/2024.
//

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"
#include "context.h"

void parser_init(Parser *parser, Context *context) {
    parser->context = context;
    parser->lexer = &context->lexer;
    parser->token_list = token_list_create(16);
    parser->current_token = nullptr;

    Token *token;
    do {
        token = lex(parser->lexer);
        token_list_add(parser->token_list, token);
    } while (token->type != TOKEN_EOF);
}

bool parser_is_initialized(Parser *parser) {
    return parser != nullptr && lexer_is_initialized(parser->lexer);
}


void advance(Parser *parser) {
    parser->current_token = token_list_next(parser->token_list);
}

Token *peek(Parser *parser) {
    size_t next = parser->token_list->current_index;
    return token_list_at(parser->token_list, next);
}

bool expect(Parser *parser, TokenType expected) {
    if (MATCH(parser, expected)) {
        return true;
    }
    // TODO: proper error reporting
    fprintf(stderr, "Error: Unexpected token '%s' expected '%s'\n",
            token_type_to_string(peek(parser)->type),
            token_type_to_string(expected));

    return false;
}

unsigned char matches(Parser *parser, ...) {
    va_list valist;
    va_start(valist, parser);

    int matched = 0;
    int token_type;

    while ((token_type = va_arg(valist, int)) != -1) {
        if (peek(parser)->type == TOKEN_EOF) {
            advance(parser);
            return token_type == TOKEN_EOF;
        }
        if ((TokenType) token_type == peek(parser)->type) {
            advance(parser);
            matched = 1;
            break;
        }
    }

    va_end(valist);
    return matched;
}

bool matches_current(Parser *parser, ...) {
    va_list valist;
    va_start(valist, parser);

    int matched = 0;
    int token_type;

    while ((token_type = va_arg(valist, int)) != -1) {
        if (parser->current_token && token_type == parser->current_token->type) {
            matched = 1;
            break;
        }
    }

    va_end(valist);
    return matched;
}

ASTNode *parse(Context *ctx) {
    ASTNode *root = parse_program(&ctx->parser);
    token_list_free(ctx->parser.token_list);
    return root;
}

ASTNode *parse_program(Parser *parser) {
    ASTNode **declarations = NULL;
    size_t capacity = 0;
    size_t count = 0;

    while (!MATCH(parser, TOKEN_EOF)) {
        ASTNode *decl = parse_decl_stmt(parser);

        if (!decl) return nullptr;

        if (count >= capacity) {
            capacity = capacity == 0 ? 4 : capacity * 2;
            declarations = realloc(declarations, sizeof(ASTNode *) * capacity);
            if (!declarations) {
                fprintf(stderr, "Memory allocation failed for declarations.\n");
                return nullptr;
            }
        }

        declarations[count++] = decl;
    }

    ASTNode *program = create_block(declarations, count);
    // Ensure the last token is EOF
    if (!CURRENT(parser, TOKEN_EOF)) {
        fprintf(stderr, "Expected End Of File, got '%s'\n",
                token_type_to_string(parser->current_token->type));
    }

    return program;
}

ASTNode *parse_fn_decl_stmt(Parser *parser) {
    expect(parser, TOKEN_IDENTIFIER);
    char *func_name = strdup(parser->current_token->str_value);

    expect(parser, TOKEN_LPAREN);

    // Parse parameter list if present
    ASTNodeList *params = create_ast_node_list();

    do {
        // only id and comma are allowed in param list
        // TODO: dynamic arguments '...'
        if (!MATCH(parser, TOKEN_IDENTIFIER, TOKEN_COMMA)) {
            if (peek(parser)->type != TOKEN_RPAREN) {
                return nullptr;
            }
        }

        if (CURRENT(parser, TOKEN_IDENTIFIER)) {
            char *param_name = strdup(parser->current_token->str_value);
            ASTNode *param_node = create_ast(AST_SYMBOL, create_string_value(param_name));
            ast_node_list_add(params, param_node);
        }

    } while (!MATCH(parser, TOKEN_RPAREN));

    expect(parser, TOKEN_LBRACE);
    ASTNode *body = parse_block_stmt(parser);

    ASTNode *func_node = create_fn_decl_stmt(func_name, params, body);

    return func_node;
}

ASTNode *parse_var_decl_stmt(Parser *parser) {
    expect(parser, TOKEN_IDENTIFIER);
    char *id = strdup(parser->current_token->str_value);
    expect(parser, TOKEN_EQUALS);
    ASTNode *node = create_ast(AST_VAR_DECL, nullptr);
    node->var_decl_expr.identifier = id;
    node->var_decl_expr.value = parse_expression(parser);
    expect(parser, TOKEN_SEMICOLON);

    return node;
}

ASTNode *parse_statement(Parser *parser) {
    if (MATCH(parser, TOKEN_LET)) {
        return parse_var_decl_stmt(parser);
    } else if (MATCH(parser, TOKEN_IF)) {
        return parse_if_stmt(parser);
    }
    if (MATCH(parser, TOKEN_FOR)) {
        return parse_for_stmt(parser);
    }
    if (MATCH(parser, TOKEN_LOOP)) {
        return parse_loop_stmt(parser);
    }
    if (MATCH(parser, TOKEN_BREAK)) {
        return parse_break_stmt(parser);
    }
    if (MATCH(parser, TOKEN_RETURN)) {
        return parse_return_stmt(parser);
    }
    if (MATCH(parser, TOKEN_LBRACE)) {
        return parse_block_stmt(parser);
    }
    // Default: parse an expression statement
    return parse_expression_stmt(parser);
}

ASTNode *parse_decl_stmt(Parser *parser) {
    if (MATCH(parser, TOKEN_FN)) {
        return parse_fn_decl_stmt(parser);
    } else if (MATCH(parser, TOKEN_LET)) {
        return parse_var_decl_stmt(parser);
    } else {
        // todo Namespaces and Classes
        return parse_statement(parser);
    }
}

ASTNode *parse_expression_stmt(Parser *parser) {
    ASTNode *expr = parse_expression(parser);
    expect(parser, TOKEN_SEMICOLON);
    // need to wrap this in an expression_stmt since we will need it in the compilation
    return create_expression_stmt(expr);
}

ASTNode *parse_block_stmt(Parser *parser) {
    ASTNode **statements = nullptr;
    size_t capacity = 0;
    size_t count = 0;

    while (!is_parse_end(parser) && !MATCH(parser, TOKEN_RBRACE)) {
        ASTNode *stmt = parse_statement(parser);
        if (count >= capacity) {
            capacity = capacity == 0 ? 4 : capacity * 2;
            statements = realloc(statements, sizeof(ASTNode *) * capacity);
            if (!statements) {
                fprintf(stderr, "Memory allocation failed for block statements.\n");
                return nullptr;
            }
        }

        statements[count++] = stmt;
    }
    ASTNode *block = create_block(statements, count);

    // if we did hit an RBRACE before EOF
    if (!CURRENT(parser, TOKEN_RBRACE)) {
        fprintf(stderr, "Error: reached early EOF\n");
        return nullptr;
    }


    return block;
}

ASTNode *parse_if_stmt(Parser *parser) {
    ASTNode *condition = parse_expression(parser);

    if (!condition) {
        fprintf(stderr, "Error: expected a condition in if statement.\n");
        return nullptr;
    }

    ASTNode *then_branch = parse_statement(parser);

    if (!then_branch) {
        fprintf(stderr, "Warning: encountered an empty if statement.\n");
    }

    ASTNode *else_branch = NULL;

    if (MATCH(parser, TOKEN_ELSE)) {
        else_branch = parse_statement(parser);
    }

    return create_if_stmt(condition, then_branch, else_branch);
}

ASTNode *parse_loop_stmt(Parser *parser) {
    return nullptr;
}

ASTNode *parse_for_stmt(Parser *parser) {
    expect(parser, TOKEN_IDENTIFIER);

    // loop variable name
    const char *identifier = strdup(parser->current_token->str_value);

    expect(parser, TOKEN_IN);

    ASTNode *range = parse_range_expr(parser);
    if (!range) {
        fprintf(stderr, "Invalid range expression in for loop.\n");
        return nullptr;
    }

    ASTNode *body = parse_statement(parser);
    if (!body) {
        fprintf(stderr, "Expected body after for loop.\n");
        return nullptr;
    }

    return create_for_stmt(identifier, range, body);
}

ASTNode *parse_break_stmt(Parser *parser) {
    ASTNode *node = create_ast(AST_BREAK, nullptr);
    return node;
}

ASTNode *parse_return_stmt(Parser *parser) {
    ASTNode *ret = create_ast(AST_RETURN, nullptr);
    ASTNode *value = nullptr;
    if (!CURRENT(parser, TOKEN_SEMICOLON)) {
        value = parse_expression(parser);
    }

    expect(parser, TOKEN_SEMICOLON); // eat ';'

    ret->return_stmt.value = value;

    return ret;
}

ASTNode *parse_range_expr(Parser *parser) {
    ASTNode *start = parse_expression(parser);

    expect(parser, TOKEN_DOTDOT);

    ASTNode *end = parse_expression(parser);

    return create_range_expr(start, end);
}

ASTNode *parse_expression(Parser *parser) {
    return parse_assignment_expression(parser);
}

ASTNode *parse_assignment_expression(Parser *parser) {
    ASTNode *left = parse_ternary_expression(parser);
    if (MATCH(parser, TOKEN_EQUALS)) {
        TokenType operator = parser->current_token->type;
        ASTNode *right = parse_assignment_expression(parser);
        ASTNode *node = create_ast(AST_ASSIGN, NULL);
        node->assignment_expr.left = left;
        node->assignment_expr.operator = operator;
        node->assignment_expr.right = right;
        return node;
    }
    return left;
}

ASTNode *parse_ternary_expression(Parser *parser) {
    ASTNode *condition = parse_logical_or_expression(parser);
    if (MATCH(parser, TOKEN_QUESTION)) {
        advance(parser);
        ASTNode *true_expr = parse_expression(parser);
        expect(parser, TOKEN_COLON);
        advance(parser);
        ASTNode *false_expr = parse_ternary_expression(parser);
        ASTNode *node = create_ast(AST_TERNARY_OP, NULL);
        node->ternary_op_expr.condition = condition;
        node->ternary_op_expr.true_expr = true_expr;
        node->ternary_op_expr.false_expr = false_expr;
        return node;
    }
    return condition;
}

ASTNode *parse_logical_or_expression(Parser *parser) {
    ASTNode *node = parse_logical_and_expression(parser);

    while (MATCH(parser, TOKEN_OR)) {
        TokenType operator = parser->current_token->type;

        ASTNode *right = parse_logical_and_expression(parser);

        ASTNode *new_node = create_ast(AST_COMPARE, NULL);
        new_node->binary_op_expr.left = node;
        new_node->binary_op_expr.operator = operator;
        new_node->binary_op_expr.right = right;

        node = new_node;
    }

    return node;
}

ASTNode *parse_logical_and_expression(Parser *parser) {
    ASTNode *node = parse_equality_expression(parser);

    while (MATCH(parser, TOKEN_AND)) {
        TokenType operator = parser->current_token->type;

        ASTNode *right = parse_equality_expression(parser);

        ASTNode *new_node = create_ast(AST_BINARY_OP, NULL);
        new_node->binary_op_expr.left = node;
        new_node->binary_op_expr.operator = operator;
        new_node->binary_op_expr.right = right;

        node = new_node;
    }

    return node;
}

ASTNode *parse_equality_expression(Parser *parser) {
    ASTNode *node = parse_relational_expression(parser);

    while (MATCH(parser, TOKEN_EQ, TOKEN_NEQ)) {
        TokenType operator = parser->current_token->type;

        ASTNode *right = parse_relational_expression(parser);

        ASTNode *new_node = create_ast(AST_COMPARE, NULL);
        new_node->binary_op_expr.left = node;
        new_node->binary_op_expr.operator = operator;
        new_node->binary_op_expr.right = right;

        node = new_node;
    }

    return node;
}

ASTNode *parse_relational_expression(Parser *parser) {
    ASTNode *node = parse_additive_expression(parser);

    while (MATCH(parser, TOKEN_LT, TOKEN_GT, TOKEN_LTE, TOKEN_GTE)) {
        TokenType operator = parser->current_token->type;
        ASTNode *right = parse_additive_expression(parser);

        ASTNode *new_node = create_ast(AST_COMPARE, NULL);
        new_node->binary_op_expr.left = node;
        new_node->binary_op_expr.operator = operator;
        new_node->binary_op_expr.right = right;

        node = new_node;
    }

    return node;
}

ASTNode *parse_additive_expression(Parser *parser) {
    ASTNode *lhs = parse_multiplicative_expression(parser);
    if (lhs == NULL) return NULL;

    while (MATCH(parser, TOKEN_PLUS, TOKEN_MINUS) == 1) {
        ASTNode *node = create_ast(AST_BINARY_OP, NULL);
        node->binary_op_expr.left = lhs;
        node->binary_op_expr.operator = parser->current_token->type;
        ASTNode *rhs = parse_multiplicative_expression(parser);
        if (rhs == NULL) {
            free_ast_node(node);
            return nullptr;
        }

        node->binary_op_expr.right = rhs;
        lhs = node;
    }

    return lhs;
}

ASTNode *parse_multiplicative_expression(Parser *parser) {
    ASTNode *lhs = parse_unary_expression(parser);
    if (lhs == NULL) return NULL;

    while (MATCH(parser, TOKEN_ASTERISK, TOKEN_SLASH)) {
        ASTNode *node = create_ast(AST_BINARY_OP, nullptr);
        node->binary_op_expr.left = lhs;
        node->binary_op_expr.operator = parser->current_token->type;
        ASTNode *rhs = parse_unary_expression(parser);
        if (rhs == NULL) {
            free_ast_node(node);
            return nullptr;
        }

        node->binary_op_expr.right = rhs;
        lhs = node;
    }

    return lhs;
}

ASTNode *parse_unary_expression(Parser *parser) {
    ASTNode *node = NULL;

    while (MATCH(parser, TOKEN_MINUS, TOKEN_BANG)) {
        TokenType operator = parser->current_token->type;
        ASTNode *unary_op = create_ast(AST_UNARY_OP, NULL);
        unary_op->unary_op_expr.operator = operator;
        unary_op->unary_op_expr.operand = node != NULL ? node : parse_unary_expression(parser);

        node = unary_op;
    }

    return node != NULL ? node : parse_primary_expression(parser);
}

ASTNode *parse_call_expression(Parser *parser, ASTNode *callee) {
    ASTNodeList *arguments = create_ast_node_list();

    do {
        ASTNode *param_node = parse_expression(parser);

        if (param_node) {
            ast_node_list_add(arguments, param_node);
        } else {
            break;
        }

    } while (MATCH(parser, TOKEN_COMMA));

    if (!MATCH(parser, TOKEN_RPAREN)) {
        fprintf(stderr, "Error: Expected closing parenthesis in function call '%s'", callee->value->str_value);
        exit(EXIT_FAILURE);
    }

    ASTNode *node = create_ast(AST_CALL, nullptr);
    node->call_expr.callee = callee;
    node->call_expr.arguments = arguments;

    return node;
}

ASTNode *parse_primary_expression(Parser *parser) {
    ASTNode *node;

    // TODO: merge integer and float into one AST_NUMBER node
    if (MATCH(parser, TOKEN_INTEGER)) {
        node = create_ast(AST_INTEGER, create_int_value(parser->current_token->int_value));
    } else if (MATCH(parser, TOKEN_FLOAT)) {
        node = create_ast(AST_FLOAT, create_float_value(parser->current_token->float_value));
    } else if (MATCH(parser, TOKEN_TRUE)) {
        node = create_ast(AST_BOOL, create_bool_value(parser->current_token->bool_value));
    } else if (MATCH(parser, TOKEN_STRING)) {
        node = create_ast(AST_STRING, create_string_value(parser->current_token->str_value));
    }
        // TODO: function calls
    else if (MATCH(parser, TOKEN_IDENTIFIER)) {
        char* id = parser->current_token->str_value;

        node = create_ast(AST_SYMBOL, create_string_value(id));

    } else if (MATCH(parser, TOKEN_LPAREN)) {
        node = parse_expression(parser);
        expect(parser, TOKEN_RPAREN);
    } else {
        return nullptr;
    }

    // Call expressions and member access

    if (MATCH(parser, TOKEN_LPAREN)) {
        node = parse_call_expression(parser, node);
    }

    // while (MATCH(parser, TOKEN_LPAREN, TOKEN_DOT)) {
    //     if (CURRENT(parser, TOKEN_LPAREN)) {
    //         node = parse_call_expression(parser, node);
    //     } // member access next
    //     else if (MATCH(parser, TOKEN_DOT)) {
    //         return nullptr;
    //     }
    // }
    // TODO: other primary expressions

    return node;
}

unsigned char is_parse_end(Parser *parser) {
    // current_token is NULL only in the start of parsing phase
    return parser->current_token != nullptr && CURRENT(parser, TOKEN_EOF);
}

