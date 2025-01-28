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

Statement parse_statement(Token *tokens, size_t num_tokens) {
    if (!num_tokens) (Statement) {.type=None};
    if (tokens[0].ttype == Varname) {
        return parse_define_assign_statement(tokens, num_tokens);
    } else {
        printf("Unknown statement type (first token type is %s).\n", ttype_as_str(tokens[0].ttype));
        exit(1);
    }
}
