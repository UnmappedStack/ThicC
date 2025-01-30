#pragma once
#include <stdint.h>

typedef struct ASTBranch ASTBranch;

typedef struct {
    ASTBranch *left_val;
    Token op;
    ASTBranch *right_val;
} BinOpNode;

typedef struct {
    TokenType op;
    ASTBranch *val;
} UnaryNode;

typedef enum {
    BinOp,
    UnaryOp,
    Number,
    Var,
} ASTBranchType;

struct ASTBranch {
    ASTBranchType type;
    union {
        BinOpNode binop;
        UnaryNode unaryop;
        uint64_t number;
        char *var;
    };
}; 

void print_ast(ASTBranch *ast, size_t depth);
