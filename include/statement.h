#pragma once
#include <lexer.h>
#include <ast.h>
#include <stddef.h>

typedef struct {
    TokenType size; // Set to NewLine if it's just an assign statement
    char *name;
    ASTBranch *val;
} DefineAssignStatement;

typedef enum {
    DefineAssign,
    None,
} StatementType;

typedef struct {
    StatementType type;
    union {
        DefineAssignStatement define_assign;
    };
} Statement;

Statement parse_statement(Token *tokens, size_t num_tokens);
