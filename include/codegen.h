#pragma once
#include <stddef.h>
#include <parser.h>

typedef struct {
    size_t rgstr;
    char *var;
} VarRegPair;

void generate_qbe(FunctionSignature *functab, size_t num_functions, int outfd);
