#include <statement.h>
#include <expression.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

static Statement parse_define_assign_statement(Token *tokens, size_t num_tokens) {
    TokenType size;
    ASTBranch *val;
    if (tokens[1].ttype == Colon) {
        size = tokens[2].ttype;
        assert(tokens[3].ttype == Equ && "tokens[3] in define statement must be Equ (=)");
        val = parse_expression(&tokens[4], num_tokens - 4); 
    } else {
        size = NewLine; // is a statement
        assert(tokens[1].ttype == Equ && "tokens[1] in assign statement must be Equ (=)");
        val = parse_expression(&tokens[2], num_tokens - 2); 
    }
    return (Statement) {
        .type = DefineAssign,
        .define_assign = (DefineAssignStatement) {
            .size = size,
            .name = tokens[0].strval,
            .val = val,
        },
    };
}

// Returns length of arg list and puts arg list into buf
static size_t parse_expression_list(Token *tokens, size_t num_tokens, ASTBranch ***buf) {
    // FIXME: Nested function calls likely won't work.
    ASTBranch **ret = (ASTBranch**) malloc(sizeof(ASTBranch*));
    size_t num_args = 0;
    size_t min_tok = 2;
    size_t max_tok = 2;
    for (size_t i = 2; i < num_tokens - 2; i++) {
        if (tokens[i].ttype == Comma || i == 0) {
            ret[num_args] = parse_expression(&tokens[min_tok], max_tok - min_tok);
            ret = realloc(ret, sizeof(ret[0]) * (num_args + 1));
            num_args++;
            min_tok = i;
            max_tok = i;
        } else
            max_tok++;
    }
    *buf = ret;
    return num_args;
}

static Statement parse_return_statement(Token *tokens, size_t num_tokens) {
    return (Statement) {
        .type = Ret,
        .ret = (ReturnStatement) {
            .val = parse_expression(&tokens[1], num_tokens - 1),
        },
    };
}

static Statement parse_function_call_statement(Token *tokens, size_t num_tokens) {
    ASTBranch **args;
    size_t num_args = parse_expression_list(tokens, num_tokens, &args);
    return (Statement) {
        .type = FunctionCall,
        .function_call = (FunctionCallStatement) {
            .name = tokens[0].strval,
            .args = args,
            .num_args = num_args,
        },
    };
}

Statement parse_statement(Token *tokens, size_t num_tokens) {
    if (!num_tokens) (Statement) {.type=None};
    if (tokens[0].ttype == Varname) {
        return parse_define_assign_statement(tokens, num_tokens);
    } else if (tokens[0].ttype == Identifier && tokens[1].ttype == LParen) {
        return parse_function_call_statement(tokens, num_tokens);
    } else if (tokens[0].ttype == Return) {
        return parse_return_statement(tokens, num_tokens);
    } else {
        printf("Unknown statement type (first token type is %s).\n", ttype_as_str(tokens[0].ttype));
        exit(1);
    }
}
