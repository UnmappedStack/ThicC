#include <codegen.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TABS(x) \
    do { \
        for (size_t tab = 0; tab < x; tab++) { \
            fprintf(f, "\t"); \
        } \
    } while (0)

// Because when have global variables ever not been a great idea...
VarRegPair *regmap_current_fn;
size_t regmap_len;
FILE *f;

#define push_regmap(name, reg) \
    do { \
        regmap_current_fn[regmap_len].rgstr= reg; \
        regmap_current_fn[regmap_len].var = (char*) malloc(strlen(name) + 1); \
        strcpy(regmap_current_fn[regmap_len].var, name); \
        regmap_len++; \
        regmap_current_fn = realloc(regmap_current_fn, (regmap_len + 1) * sizeof(VarRegPair)); \
    } while (0)

static size_t var_to_reg(char *name) {
    for (size_t i = 0; i < regmap_len; i++) {
        if (!strcmp(regmap_current_fn[i].var, name)) return regmap_current_fn[i].rgstr;
    }
    size_t current_regmap_len = regmap_len;
    push_regmap(name, regmap_len);
    return current_regmap_len;
}

uint64_t typemap[][2] = {
    {UQword, 'l'},
};

static char convert_type(TokenType tok) {
    size_t typemap_len = sizeof(typemap) / sizeof(typemap[0]);
    for (size_t i = 0; i < typemap_len; i++) {
        if (typemap[i][0] == tok) return typemap[i][1];
    }
    printf("Unknown type (tok = %s)\n", ttype_as_str(tok));
    exit(1);
}

void codegen_ast(ASTBranch *ast, size_t depth, char *final_reg, char type) {
    if (ast->type == Number) {
        TABS(depth); fprintf(f, "%%%s =%c copy %llu\n", final_reg, type, ast->number);
    } else if (ast->type == BinOp) {
        char next[3];
        char after_next[3];
        size_t current = atoi(&final_reg[1]);
        sprintf(next, "s%llu", current + 1);
        sprintf(after_next, "s%llu", current + 2);
        codegen_ast(ast->binop.left_val, depth, next, type);
        codegen_ast(ast->binop.right_val, depth, after_next, type);
        if (ast->binop.op.ttype == Mul) {
            TABS(depth); fprintf(f, "%%%s =%c mul %%%s, %%%s\n", final_reg, type, next, after_next);
        } else if (ast->binop.op.ttype == Add) {
            TABS(depth); fprintf(f, "%%%s =%c add %%%s, %%%s\n", final_reg, type, next, after_next);
        } else {
            printf("Unknown AST operation.\n");
            exit(1);
        }
    } else if (ast->type == UnaryOp) {
        char next[3];
        size_t current = atoi(&final_reg[1]);
        sprintf(next, "s%llu", current + 1);
        codegen_ast(ast->unaryop.val, depth, next, type);
        if (ast->unaryop.op == Not) {
            TABS(depth); fprintf(f, "%%%s =%c add %%%s, 0\n", final_reg, type, next);
        } else {
            printf("Unknown AST operation.\n");
        }
    } else {
        printf("Unknown AST node format.\n");
        exit(1);
    }
}

void codegen_defineassign(Statement statement, size_t depth) {
    char type = (statement.define_assign.size == NewLine) ? 'l' : convert_type(statement.define_assign.size);
    char buf[3];
    sprintf(buf, "r%llu", var_to_reg(statement.define_assign.name));
    codegen_ast(statement.define_assign.val, depth, buf, type);
}

void codegen_statements(Statement *statements, size_t num_statements, size_t depth) {
    for (size_t i = 0; i < num_statements; i++) {
        if (statements[i].type == DefineAssign) codegen_defineassign(statements[i], depth);
        else {
            printf("Invalid statement type.\n");
            exit(1);
        }
    }
}

void generate_qbe(FunctionSignature *functab, size_t num_functions, int outfd) {
    printf("Number of functions: %llu\n", num_functions);
    remove("out.ssa");
    f = fopen("out.ssa", "a");
    if (f == NULL) {
        printf("Failed to open SSA file to write to.\n");
        exit(1);
    }
    for (size_t i = 0; i < num_functions; i++) {
        regmap_current_fn = (VarRegPair*) malloc(sizeof(VarRegPair));
        regmap_len = 0;
        fprintf(f, "export function %c $%s(env %%e, ", 
                convert_type(functab[i].ret), functab[i].name);
        for (size_t arg = 0; arg < functab[i].num_args; arg++) {
            fprintf(f, "%c %%r%llu", convert_type(functab[i].args[arg].type), var_to_reg(functab[i].args[arg].name));
            if (arg + 1 != functab[i].num_args) fprintf(f, ", ");
        }
        fprintf(f, ") {\n@start\n");
        codegen_statements(functab[i].statements, functab[i].num_statements, 1);
        fprintf(f, "\tret 0\n}\n");
    }
    fclose(f);
}
