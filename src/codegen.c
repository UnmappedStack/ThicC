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
char **string_literals;
size_t num_strlit;
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
    } else if (ast->type == Var) {
        TABS(depth); fprintf(f, "%%%s =%c copy %%r%llu\n", final_reg, type, var_to_reg(ast->var));
    } else if (ast->type == Str) {
        string_literals[num_strlit] = (char*) malloc(strlen(ast->strlit) + 1);
        strcpy(string_literals[num_strlit], ast->strlit);
        TABS(depth); fprintf(f, "%%%s =%c copy $strlit%llu\n", final_reg, type, num_strlit);
        num_strlit++;
        string_literals = realloc(string_literals, (num_strlit + 1) * sizeof(char*));
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

void codegen_return(Statement statement, size_t depth, TokenType type) {
    codegen_ast(statement.ret.val, depth, "retval", convert_type(type));
    TABS(depth); fprintf(f, "ret %retval\n");
}

int codegen_statements(Statement *statements, size_t num_statements, size_t depth, TokenType ret_type);
void codegen_scope(Statement statement, size_t depth, TokenType type) {
    TABS(depth); fprintf(f, "# Begin scope {\n");
    codegen_statements(statement.scope.statements, statement.scope.num_statements, depth + 1, type);
    TABS(depth); fprintf(f, "# } End scope\n");
}

void codegen_functioncall(Statement statement, size_t depth) {
    for (int64_t arg = statement.function_call.num_args - 1; arg >= 0; arg--) {
        char buf[6];
        snprintf(buf, 6, "a%llu", arg);
        codegen_ast(statement.function_call.args[arg], depth, buf, 'l');
    }
    TABS(depth); fprintf(f, "call $%s(", statement.function_call.name);
    for (int64_t arg = statement.function_call.num_args - 1; arg >= 0; arg--) {
        fprintf(f, "l %%a%llu%s", arg, (arg == 0) ? "" : ", ");
    }
    fprintf(f, ")\n");
}

// returns 1 if last statement was `ret`, otherwise 0
int codegen_statements(Statement *statements, size_t num_statements, size_t depth, TokenType ret_type) {
    for (size_t i = 0; i < num_statements; i++) {
        if (statements[i].type == DefineAssign) codegen_defineassign(statements[i], depth);
        else if (statements[i].type == FunctionCall) codegen_functioncall(statements[i], depth);
        else if (statements[i].type == Scope) codegen_scope(statements[i], depth, ret_type);
        else if (statements[i].type == Ret) {
            codegen_return(statements[i], depth, ret_type);
            if (i == num_statements - 1) return 1;
        }
        else {
            printf("Invalid statement type.\n");
            exit(1);
        }
    }
}

void generate_qbe(FunctionSignature *functab, size_t num_functions, int outfd) {
    printf("Number of functions: %llu\n", num_functions);
    remove("out.ssa");
    f = fopen("out.ssa", "a+");
    if (f == NULL) {
        printf("Failed to open SSA file to write to.\n");
        exit(1);
    }
    num_strlit = 0;
    string_literals = (char**) malloc(sizeof(char*));
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
        int ret_done = codegen_statements(functab[i].statements, functab[i].num_statements, 1, functab[i].ret);
        if (ret_done)
            fprintf(f, "}\n");
        else
            fprintf(f, "\tret 0\n}\n");
    }
    // This is a truly horrible, hacky, terrible solution. Ew and fuck this. (TODO)
    fseek(f, 0L, SEEK_END);
    size_t flen = ftell(f);
    fseek(f, 0L, SEEK_SET);
    char *current = (char*) malloc(flen + 1);
    fread(current, flen, 1, f);
    fclose(f);
    remove("out.ssa");
    f = fopen("out.ssa", "a");
    fprintf(f, "# Generated from Thic code with ThicC. #\n");
    for (size_t strlit = 0; strlit < num_strlit; strlit++) {
        fprintf(f, "data $strlit%llu = { b \"%s\", b 0 }\n", strlit, string_literals[strlit]);
    }
    fprintf(f, "\n%s", current);
    fclose(f);
}
