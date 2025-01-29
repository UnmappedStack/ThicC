#pragma once
#include <stddef.h>
#include <parser.h>

void generate_qbe(FunctionSignature *functab, size_t num_functions, int outfd);
