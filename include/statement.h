#pragma once
#include <lexer.h>
#include <ast.h>
#include <stddef.h>

typedef struct {
    TokenType size; // Set to NewLine if it's just an assign statement
    char *name;
    ASTBranch *val;
} DefineAssignStatement;

typedef struct {
    char *name;
    ASTBranch **args;
    size_t num_args;
} FunctionCallStatement;

typedef enum {
    DefineAssign,
    FunctionCall,
    None,
} StatementType;

typedef struct {
    StatementType type;
    union {
        DefineAssignStatement define_assign;
        FunctionCallStatement function_call;
    };
} Statement;

Statement parse_statement(Token *tokens, size_t num_tokens);
