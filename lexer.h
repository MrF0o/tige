//
// Created by fathi on 10/19/2024.
//

#ifndef TIGE_LEXER_H
#define TIGE_LEXER_H

#include <stddef.h>
#include <stdint.h>
#include "error.h"

// Token types
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,      // Added float token type
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_DOTDOT,    // ..
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_EQUALS,    // =
    TOKEN_PLUS,      // +
    TOKEN_MINUS,     // -
    TOKEN_ASTERISK,  // *
    TOKEN_SLASH,     // /
    TOKEN_GT,        // >
    TOKEN_LT,        // <
    TOKEN_GTE,       // >=
    TOKEN_LTE,       // <=
    TOKEN_EQ,        // ==
    TOKEN_NEQ,       // !=
    TOKEN_QUESTION,  // ?
    TOKEN_BANG,
    TOKEN_SCOPE,     // ::
    TOKEN_AND,       // && or AND
    TOKEN_OR,        // || or OR
    TOKEN_TRUE,
    TOKEN_FALSE,

    // Keywords
    TOKEN_CLASS,
    TOKEN_PUBLIC,
    TOKEN_PRIVATE,
    TOKEN_NAMESPACE,
    TOKEN_FN,
    TOKEN_LET,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_LOOP,
    TOKEN_BREAK,
    TOKEN_RETURN,
    TOKEN_THIS,
    TOKEN_ERROR,
    TOKEN_NONE,
} TokenType;


// Token structure
typedef struct {
    TokenType type;
    union {
        char *str_value;
        int64_t int_value;
        double float_value;
        bool bool_value;
        void *value;
    };
    int line;
    int column;
} Token;

// Lexer structure
typedef struct {
    const char *source;
    size_t source_len;
    size_t position;
    size_t line;
    size_t column;
    char current;
    ErrorList* error_list;
} Lexer;

typedef struct TokenList {
    Token **tokens;
    size_t capacity;
    size_t size;
    size_t current_index;
} TokenList;

TokenList *token_list_create(size_t initial_capacity);

void token_list_add(TokenList *list, Token *token);

void token_list_free(TokenList *list);

Token* token_list_current(TokenList* list);
Token* token_list_at(TokenList* list, size_t index);
Token* token_list_next(TokenList* list);
Token* token_list_prev(TokenList* list);

void lexer_init(Lexer *lexer, const char *source);
bool lexer_is_initialized(Lexer *lexer);
Token *lex(Lexer *lexer);

const char *token_type_to_string(TokenType type);

void token_free(Token *token);

int token_is_type(const Token *token, TokenType type);

Token *create_token(TokenType type, const char *value, int line, int column);

Token *token_create_string(TokenType type, const char *value, int line, int column);

Token *token_create_int(TokenType type, long long value, int line, int column);

Token *token_create_float(TokenType type, double value, int line, int column);

// Error handling structure
typedef struct {
    int line;
    int column;
    char *message;
} LexerError;

const LexerError *lexer_get_error(const Lexer *lexer);

// Debug functions
#ifdef LEXER_DEBUG

void token_print(const Token *token);

char *lexer_get_state(const Lexer *lexer);

#endif

// Token type categories for easier classification
static inline int token_is_operator(TokenType type) {
    return type >= TOKEN_PLUS && type <= TOKEN_NEQ;
}

static inline int token_is_keyword(TokenType type) {
    return type >= TOKEN_CLASS && type <= TOKEN_THIS;
}

static inline int token_is_literal(TokenType type) {
    return type == TOKEN_INTEGER || type == TOKEN_FLOAT || type == TOKEN_STRING;
}

static inline int token_is_numeric(TokenType type) {
    return type == TOKEN_INTEGER || type == TOKEN_FLOAT;
}

// Maximum lengths and limits
#define MAX_IDENTIFIER_LENGTH 255
#define MAX_STRING_LENGTH 4096
#define MAX_NUMBER_LENGTH 100

#endif // TIGE_LEXER_H