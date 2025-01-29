#include <codegen.h>
#include <stdio.h>
#include <stdlib.h>

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

void generate_qbe(FunctionSignature *functab, size_t num_functions, int outfd) {
    printf("Number of functions: %llu\n", num_functions);
    for (size_t i = 0; i < num_functions; i++) {
        printf("export function %c $%s(env %%e, ", 
                convert_type(functab[i].ret), functab[i].name);
        for (size_t arg = 0; arg < functab[i].num_args; arg++) {
            printf("%c %%%s", convert_type(functab[i].args[arg].type), functab[i].args[arg].name);
            if (arg + 1 != functab[i].num_args) printf(", ");
        }
        printf(") {\n\n}\n");
    }
}
