//
// Created by fathi on 10/19/2024.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "lexer.h"

// Keyword mapping
typedef struct {
    const char *keyword;
    TokenType type;
} Keyword;

static Keyword keywords[] = {
        {"class",     TOKEN_CLASS},
        {"public",    TOKEN_PUBLIC},
        {"private",   TOKEN_PRIVATE},
        {"namespace", TOKEN_NAMESPACE},
        {"fn",        TOKEN_FN},
        {"let",       TOKEN_LET},
        {"if",        TOKEN_IF},
        {"else",      TOKEN_ELSE},
        {"for",       TOKEN_FOR},
        {"in",        TOKEN_IN},
        {"loop",      TOKEN_LOOP},
        {"break",     TOKEN_BREAK},
        {"return",    TOKEN_RETURN},
        {"this",      TOKEN_THIS},
        {"and",       TOKEN_AND},
        {"or",        TOKEN_OR},
        {"true",      TOKEN_TRUE},
        {"false",     TOKEN_FALSE},
        {NULL,        TOKEN_EOF}  // sentinel
};

TokenList *token_list_create(size_t initial_capacity) {
    TokenList *list = malloc(sizeof(TokenList));
    list->tokens = malloc(sizeof(Token *) * initial_capacity);
    list->capacity = initial_capacity;
    list->size = 0;
    list->current_index = 0;
    return list;
}

void token_list_add(TokenList *list, Token *token) {
    if (list->size >= list->capacity) {
        list->capacity *= 2;
        list->tokens = realloc(list->tokens, sizeof(Token *) * list->capacity);
    }
    list->tokens[list->size++] = token;
}

Token *token_list_current(TokenList *list) {
    if (list->current_index < list->size) {
        return list->tokens[list->current_index];
    } else {
        return NULL;
    }
}

Token *token_list_at(TokenList *list, size_t index) {
    if (index < list->size) {
        return list->tokens[index];
    }
    return list->tokens[list->size - 1];
}

Token *token_list_next(TokenList *list) {
    if (list->current_index + 1 < list->size) {
        Token *tok = list->tokens[list->current_index];
        list->current_index++;

        return tok;
    }

    return create_token(TOKEN_EOF, nullptr, -1, -1);
}

Token *token_list_prev(TokenList *list) {
    if (list->current_index > 0) {
        list->current_index--;
    }
    // Optional: Handle case where you're at the start
}

void token_list_free(TokenList *list) {
    for (size_t i = 0; i < list->size; i++) {
        token_free(list->tokens[i]);
    }
    free(list->tokens);
    free(list);
}

// Helper functions
void lexer_init(Lexer *lexer, const char *source) {
    lexer->source = source;
    lexer->source_len = source != nullptr ? strlen(source) : 0;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current = source != nullptr ? source[0] : EOF;
}

bool lexer_is_initialized(Lexer *lexer) {
    return lexer != nullptr && lexer->source_len > 0;
}

void lexer_advance(Lexer *lexer) {
    if (lexer->position < lexer->source_len) {
        if (lexer->current == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
        lexer->position++;
        lexer->current = lexer->position < lexer->source_len ?
                         lexer->source[lexer->position] : '\0';
    }
}

char lexer_peek(Lexer *lexer) {
    if (lexer->position + 1 >= lexer->source_len) return '\0';
    return lexer->source[lexer->position + 1];
}

int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

void skip_whitespace(Lexer *lexer) {
    while (is_whitespace(lexer->current)) {
        lexer_advance(lexer);
    }
}

void skip_line_comment(Lexer *lexer) {
    while (lexer->current != '\n' && lexer->current != '\0') {
        lexer_advance(lexer);
    }
}

void skip_block_comment(Lexer *lexer) {
    int nesting = 1;
    lexer_advance(lexer); // skip *

    while (nesting > 0 && lexer->current != '\0') {
        if (lexer->current == '/' && lexer_peek(lexer) == '*') {
            lexer_advance(lexer);
            lexer_advance(lexer);
            nesting++;
        } else if (lexer->current == '*' && lexer_peek(lexer) == '/') {
            lexer_advance(lexer);
            lexer_advance(lexer);
            nesting--;
        } else {
            lexer_advance(lexer);
        }
    }
}

Token *lex_number(Lexer *lexer) {
    size_t start_pos = lexer->position;
    int start_col = lexer->column;
    int is_float = 0;

    while (is_digit(lexer->current)) {
        lexer_advance(lexer);
    }

    if (lexer->current == '.' && lexer_peek(lexer) != '.') {
        is_float = 1;
        lexer_advance(lexer);

        if (!is_digit(lexer->current)) {
            size_t length = lexer->position - start_pos;
            char *value = (char *) malloc(length + 1);
            strncpy(value, &lexer->source[start_pos], length);
            value[length] = '\0';
            return create_token(TOKEN_ERROR, value, lexer->line, start_col);
        }

        while (is_digit(lexer->current)) {
            lexer_advance(lexer);
        }

        if (lexer->current == 'e' || lexer->current == 'E') {
            is_float = 1;
            lexer_advance(lexer);

            if (lexer->current == '+' || lexer->current == '-') {
                lexer_advance(lexer);
            }

            if (!is_digit(lexer->current)) {
                size_t length = lexer->position - start_pos;
                char *value = (char *) malloc(length + 1);
                strncpy(value, &lexer->source[start_pos], length);
                value[length] = '\0';
                return create_token(TOKEN_ERROR, value, lexer->line, start_col);
            }

            while (is_digit(lexer->current)) {
                lexer_advance(lexer);
            }
        }
    }
    else if (lexer->current == 'e' || lexer->current == 'E') {
        is_float = 1;
        lexer_advance(lexer);

        if (lexer->current == '+' || lexer->current == '-') {
            lexer_advance(lexer);
        }

        if (!is_digit(lexer->current)) {
            size_t length = lexer->position - start_pos;
            char *value = (char *) malloc(length + 1);
            strncpy(value, &lexer->source[start_pos], length);
            value[length] = '\0';
            return create_token(TOKEN_ERROR, value, lexer->line, start_col);
        }

        while (is_digit(lexer->current)) {
            lexer_advance(lexer);
        }
    }

    size_t length = lexer->position - start_pos;
    char *value = (char *) malloc(length + 1);
    strncpy(value, &lexer->source[start_pos], length);
    value[length] = '\0';

    TokenType type = is_float ? TOKEN_FLOAT : TOKEN_INTEGER;
    return create_token(type, value, lexer->line, start_col);
}

Token *lex_string(Lexer *lexer) {
    char quote = lexer->current;
    size_t start_pos = lexer->position + 1;
    size_t start_col = lexer->column;

    lexer_advance(lexer); // skip opening quote

    while (lexer->current != quote && lexer->current != '\0') {
        lexer_advance(lexer);
    }

    size_t length = lexer->position - start_pos;
    char *value = (char *) malloc(length + 1);
    strncpy(value, &lexer->source[start_pos], length);
    value[length] = '\0';

    lexer_advance(lexer); // skip closing quote

    return create_token(TOKEN_STRING, value, lexer->line, start_col);
}

Token *lex_identifier_or_keyword(Lexer *lexer) {
    size_t start_pos = lexer->position;
    size_t start_col = lexer->column;

    while (is_alpha(lexer->current) || is_digit(lexer->current)) {
        lexer_advance(lexer);
    }

    size_t length = lexer->position - start_pos;
    char *value = (char *) malloc(length + 1);
    strncpy(value, &lexer->source[start_pos], length);
    value[length] = '\0';

    // Check if it's a keyword
    for (int i = 0; keywords[i].keyword != NULL; i++) {
        if (strcmp(value, keywords[i].keyword) == 0) {
            TokenType type = keywords[i].type;
            Token *token = create_token(type, value, lexer->line, start_col);

            // Handle boolean literals
            if (type == TOKEN_TRUE) {
                token->bool_value = true;
                token->type = TOKEN_TRUE;
            } else if (type == TOKEN_FALSE) {
                token->bool_value = false;
                token->type = TOKEN_TRUE;
            }

            return token;
        }
    }

    return create_token(TOKEN_IDENTIFIER, value, lexer->line, start_col);
}

Token *create_token(TokenType type, const char *value, int line, int column) {
    Token *token = (Token *) malloc(sizeof(Token));
    token->type = type;
    token->line = line;
    token->column = column;
    token->value = NULL;
    if (value) {
        // For float and integer tokens, store both string and numeric values
        if (type == TOKEN_FLOAT) {
            token->str_value = strdup(value);
            token->float_value = atof(value);  // Convert string to float
        } else if (type == TOKEN_INTEGER) {
            token->str_value = strdup(value);
            token->int_value = atoll(value);   // Convert string to long long
        } else {
            token->str_value = strdup(value);
        }
    } else {
        token->str_value = nullptr;
    }

    return token;
}

int token_is_type(const Token *token, TokenType type) {
    return token != NULL && token->type == type;
}

const LexerError *lexer_get_error(const Lexer *lexer) {
    return NULL;
}

Token *lex(Lexer *lexer) {
    skip_whitespace(lexer);

    if (lexer->current == '\0') {
        return create_token(TOKEN_EOF, NULL, lexer->line, lexer->column);
    }

    int current_line = lexer->line;
    int current_column = lexer->column;

    // Handle comments
    if (lexer->current == '/' && lexer_peek(lexer) == '/') {
        skip_line_comment(lexer);
        return lex(lexer);
    }
    if (lexer->current == '/' && lexer_peek(lexer) == '*') {
        skip_block_comment(lexer);
        return lex(lexer);
    }

    // Handle numbers
    if (is_digit(lexer->current)) {
        return lex_number(lexer);
    }

    // Handle strings
    if (lexer->current == '\'' || lexer->current == '"') {
        return lex_string(lexer);
    }

    // Handle identifiers and keywords
    if (is_alpha(lexer->current)) {
        return lex_identifier_or_keyword(lexer);
    }

    // Handle operators and other symbols
    char current = lexer->current;
    lexer_advance(lexer);

    switch (current) {
        case ';':
            return create_token(TOKEN_SEMICOLON, ";", current_line, current_column);
        case ':':
            if (lexer->current == ':') {
                lexer_advance(lexer);
                return create_token(TOKEN_SCOPE, "::", current_line, current_column);
            }
            return create_token(TOKEN_COLON, ":", current_line, current_column);
        case ',':
            return create_token(TOKEN_COMMA, ",", current_line, current_column);
        case '.':
            if (lexer->current == '.') {
                lexer_advance(lexer);
                return create_token(TOKEN_DOTDOT, "..", current_line, current_column);
            }
            return create_token(TOKEN_DOT, ".", current_line, current_column);
        case '(':
            return create_token(TOKEN_LPAREN, "(", current_line, current_column);
        case ')':
            return create_token(TOKEN_RPAREN, ")", current_line, current_column);
        case '{':
            return create_token(TOKEN_LBRACE, "{", current_line, current_column);
        case '}':
            return create_token(TOKEN_RBRACE, "}", current_line, current_column);
        case '+':
            return create_token(TOKEN_PLUS, "+", current_line, current_column);
        case '-':
            return create_token(TOKEN_MINUS, "-", current_line, current_column);
        case '*':
            return create_token(TOKEN_ASTERISK, "*", current_line, current_column);
        case '/':
            return create_token(TOKEN_SLASH, "/", current_line, current_column);
        case '|':
            if (lexer->current == '|') {
                lexer_advance(lexer);
                return create_token(TOKEN_OR, "||", current_line, current_column);
            }
            break;
        case '&':
            if (lexer->current == '&') {
                lexer_advance(lexer);
                return create_token(TOKEN_AND, "||", current_line, current_column);
            }
            break;
        case '=':
            if (lexer->current == '=') {
                lexer_advance(lexer);
                return create_token(TOKEN_EQ, "==", current_line, current_column);
            }
            return create_token(TOKEN_EQUALS, "=", current_line, current_column);
        case '>':
            if (lexer->current == '=') {
                lexer_advance(lexer);
                return create_token(TOKEN_GTE, ">=", current_line, current_column);
            }
            return create_token(TOKEN_GT, ">", current_line, current_column);
        case '<':
            if (lexer->current == '=') {
                lexer_advance(lexer);
                return create_token(TOKEN_LTE, "<=", current_line, current_column);
            }
            return create_token(TOKEN_LT, "<", current_line, current_column);
        case '!':
            if (lexer->current == '=') {
                lexer_advance(lexer);
                return create_token(TOKEN_NEQ, "!=", current_line, current_column);
            } else {
                lexer_advance(lexer);
                return create_token(TOKEN_BANG, "!", current_line, current_column);
            }
            break;
        case '?':
            return create_token(TOKEN_QUESTION, "?", current_line, current_column);
    }

    // Handle unknown characters
    char unknown_char[2] = {current, '\0'};
    return create_token(TOKEN_EOF, unknown_char, current_line, current_column);
}

// Token manipulation functions
void token_free(Token *token) {
    if (token != NULL) {
        if (
                (token->type == TOKEN_STRING ||
                 token->type == TOKEN_IDENTIFIER ||
                 token_is_keyword(token->type)) &&
                token->str_value != nullptr
                ) {
            free(token->str_value);
        }
        free(token);         // Free the token structure itself
    }
}

#ifdef LEXER_DEBUG

// Print token information
void print_token(const Token *token) {
    printf("Line %d, Column %d: %s", token->line, token->column, token_type_to_string(token->type));

    // Check if the token has a value to print
    if (token->value != NULL) {
        printf(" '%s'", token->value);
    }

    printf("\n");
}

const char *token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF:
            return "EOF";
        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";
        case TOKEN_INTEGER:
            return "INTEGER";
        case TOKEN_STRING:
            return "STRING";
        case TOKEN_OPERATOR:
            return "OPERATOR";
        case TOKEN_SEMICOLON:
            return "SEMICOLON";
        case TOKEN_COLON:
            return "COLON";
        case TOKEN_COMMA:
            return "COMMA";
        case TOKEN_DOT:
            return "DOT";
        case TOKEN_DOTDOT:
            return "DOTDOT";
        case TOKEN_LPAREN:
            return "LPAREN";
        case TOKEN_RPAREN:
            return "RPAREN";
        case TOKEN_LBRACE:
            return "LBRACE";
        case TOKEN_RBRACE:
            return "RBRACE";
        case TOKEN_EQUALS:
            return "EQUALS";
        case TOKEN_PLUS:
            return "PLUS";
        case TOKEN_MINUS:
            return "MINUS";
        case TOKEN_ASTERISK:
            return "ASTERISK";
        case TOKEN_SLASH:
            return "SLASH";
        case TOKEN_GT:
            return "GT";
        case TOKEN_LT:
            return "LT";
        case TOKEN_GTE:
            return "GTE";
        case TOKEN_LTE:
            return "LTE";
        case TOKEN_EQ:
            return "EQ";
        case TOKEN_NEQ:
            return "NEQ";
        case TOKEN_QUESTION:
            return "QUESTION";
        case TOKEN_SCOPE:
            return "SCOPE";
        case TOKEN_CLASS:
            return "CLASS";
        case TOKEN_PUBLIC:
            return "PUBLIC";
        case TOKEN_PRIVATE:
            return "PRIVATE";
        case TOKEN_NAMESPACE:
            return "NAMESPACE";
        case TOKEN_FN:
            return "FN";
        case TOKEN_LET:
            return "LET";
        case TOKEN_IF:
            return "IF";
        case TOKEN_ELSE:
            return "ELSE";
        case TOKEN_FOR:
            return "FOR";
        case TOKEN_IN:
            return "IN";
        case TOKEN_LOOP:
            return "LOOP";
        case TOKEN_BREAK:
            return "BREAK";
        case TOKEN_RETURN:
            return "RETURN";
        case TOKEN_THIS:
            return "THIS";
        default:
            return "UNKNOWN";
    }
}

char *lexer_get_state(const Lexer *lexer) {
    if (lexer == NULL) {
        return strdup("NULL LEXER");
    }

    // Allocate enough space for the state string
    char *state = (char *) malloc(256);
    if (state == NULL) {
        return NULL;
    }

    snprintf(state, 256,
             "Lexer State {\n"
             "  position: %zu\n"
             "  line: %zu\n"
             "  column: %zu\n"
             "  current: '%c' (0x%02X)\n"
             "}",
             lexer->position,
             lexer->line,
             lexer->column,
             lexer->current,
             (unsigned char) lexer->current);

    return state;
}


Token *prev(Lexer *lexer) {

}

#endif