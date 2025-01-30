#include <codegen.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TABS(x) \
    do { \
        for (size_t tab = 0; tab < x; tab++) { \
            printf("\t"); \
        } \
    } while (0)

// Because when have global variables ever not been a great idea...
VarRegPair *regmap_current_fn;
size_t regmap_len;

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

void codegen_defineassign(Statement statement, size_t depth) {
    TABS(depth); printf("test reg conversion: %%%llu\n", var_to_reg("val2"));
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
    for (size_t i = 0; i < num_functions; i++) {
        regmap_current_fn = (VarRegPair*) malloc(sizeof(VarRegPair));
        regmap_len = 0;
        printf("export function %c $%s(env %%e, ", 
                convert_type(functab[i].ret), functab[i].name);
        for (size_t arg = 0; arg < functab[i].num_args; arg++) {
            printf("%c %%%llu", convert_type(functab[i].args[arg].type), var_to_reg(functab[i].args[arg].name));
            if (arg + 1 != functab[i].num_args) printf(", ");
        }
        printf(") {\n@start\n");
        
        codegen_statements(functab[i].statements, functab[i].num_statements, 1);
        printf("\n}\n");
    }
}
