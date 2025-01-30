#include <parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static size_t parse_arglist_sig(Token *tokens, ArgPair **argsbuf) {
    *argsbuf = (ArgPair*) malloc(sizeof(ArgPair*));
    size_t num_args = 0;
    for (size_t i = 0; tokens[i].ttype != RParen; i++) {
        if (tokens[i].ttype == RParen) break;
        if (!is_type(tokens[i].ttype)) {
            printf("Expected type in function arg list, got something else (token type: %s)\n",
                    ttype_as_str(tokens[i].ttype));
            exit(1);
        } else if (tokens[i + 1].ttype != Varname) {
            printf("Expected variable name in function arg list, got something else (token type: %s)\n",
                    ttype_as_str(tokens[i].ttype));
            exit(1);
        } else {
            (*argsbuf)[num_args] = (ArgPair) {
                .type = tokens[i].ttype,
                .name = tokens[i + 1].strval,
            };
            num_args++;
            *argsbuf = realloc(*argsbuf, (num_args + 1) * sizeof(ArgPair));
            i += 2;
            if (tokens[i].ttype == RParen) break;
        }
    }
    return num_args;
}

static size_t parse_function_sig(Token *tokens, FunctionSignature *sigbuf) {
    size_t skip = 6; /* More added based on args but for now all that it know needs to be skipped is 6 tokens
                      * to get to the opening curly brace. */
    ArgPair *args;
    size_t num_args = parse_arglist_sig(&tokens[3], &args);
    if (num_args) skip += (num_args * 3) - 2;
    *sigbuf = (FunctionSignature) {
        .name = tokens[0].strval,
        .args = args,
        .num_args = num_args,
        .ret = tokens[skip].ttype,
    };
    return skip + 1;
}

static size_t parse_fn(Token *tokens, size_t num_tokens, Statement **statements_buf) {
    *statements_buf = (Statement*) malloc(sizeof(Statement));
    size_t num_statements = 0;
    size_t start = 0;
    size_t end = 0;
    for (size_t i = 0; i < num_tokens; i++) {
        if (tokens[i].ttype == NewLine) {
            (*statements_buf)[num_statements] = (Statement) parse_statement(&tokens[start], end - start);
            num_statements++;
            *statements_buf = realloc(*statements_buf, (num_statements + 1) * sizeof(Statement));
            start = end = i + 1;
        }
        end++;
    }
    return num_statements;
}

size_t parse_program(Token *tokens, size_t num_tokens, FunctionSignature **buf) {
    FunctionSignature *ret = (FunctionSignature*) malloc(sizeof(FunctionSignature));
    size_t num_funcs = 0;
    for (size_t i = 0; i < num_tokens; i++) {
        if (tokens[i].ttype == Identifier && tokens[i + 1].ttype == Colon) {
            FunctionSignature sigbuf;
            i += parse_function_sig(&tokens[i], &sigbuf);
            size_t current_depth = 1;
            Token *start = &tokens[i];
            i++;
            size_t len = 0;
            for (;; i++) {
                if (tokens[i].ttype == RBrace) {
                    current_depth--;
                    if (!current_depth) break;
                }
                len++;
            }
            i += len;
            sigbuf.num_statements = parse_fn(&start[2], len - 1, &sigbuf.statements);
            ret[num_funcs] = sigbuf;
            num_funcs++;
            ret = realloc(ret, (sizeof(FunctionSignature) + 1) * num_funcs);
        } else {
            printf("There's something outside of the scope of a function.\n");
            exit(1);
        }
    }
    *buf = ret;
    return num_funcs;
}
