#pragma once
#include <lexer.h>
#include <ast.h>
#include <stddef.h>

typedef struct Statement Statement;

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

typedef struct {
    ASTBranch *val;
} ReturnStatement;

typedef struct {
    Statement *statements;
    size_t num_statements;
} ScopeStatement;

typedef enum {
    DefineAssign,
    FunctionCall,
    Ret,
    Scope,
    None,
} StatementType;

struct Statement {
    StatementType type;
    union {
        DefineAssignStatement define_assign;
        FunctionCallStatement function_call;
        ReturnStatement       ret;
        ScopeStatement        scope;
    };
};

Statement parse_statement(Token *tokens, size_t num_tokens, size_t *skip);
