#pragma once
#include <lexer.h>
#include <statement.h>
#include <stddef.h>

typedef struct {
    TokenType type;
    char *name;
} ArgPair;

typedef struct {
    char *name;
    TokenType ret;
    ArgPair *args;
    size_t num_args;
    Statement *statements;
    size_t num_statements;
} FunctionSignature;

size_t parse_program(Token *tokens, size_t num_tokens, FunctionSignature **buf);
size_t parse_fn(Token *tokens, size_t num_tokens, Statement **statements_buf);
