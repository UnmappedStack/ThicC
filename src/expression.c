#include <expression.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

uint16_t priority_map[][2] = {
    {Mul, 5},
    {Div, 5},
    {Sub, 6},
    {Add, 6},
};

char *ttype_as_str(TokenType tok) {
    if (tok == Mul) return "Mul";
    else if (tok == Div) return "Div";
    else if (tok == Add) return "Add";
    else if (tok == Sub) return "Sub";
    else if (tok == Equ) return "Equals";
    else if (tok == LBrace) return "LBrace";
    else if (tok == RBrace) return "RBrace";
    else if (tok == Not) return "Not";
    else if (tok == Comma) return "Comma";
    else if (tok == LParen) return "LParen";
    else if (tok == RParen) return "RParen";
    else if (tok == UnsignedInt) return "Unsigned Int";
    else if (tok == Identifier) return "Identifier";
    else if (tok == Colon) return "Colon";
    else if (tok == Varname) return "Variable Name";
    else if (tok == UQword) return "UQWord";
    else if (tok == NewLine) return "New Line";
    else if (tok == Return) return "Return";
    else if (tok == StrLit) return "String Literal";
    else return "Unmapped token type";
}

static uint64_t get_priority(TokenType tok) {
    size_t priority_map_len = sizeof(priority_map) / sizeof(priority_map[0]);
    for (size_t i = 0; i < priority_map_len; i++) {
        if (tok == priority_map[i][0]) return priority_map[i][1];
    }
    printf("Couldn't get priority of token, not in priority map.\n");
    exit(1);
}

// Move to a new file, perhaps include/utils.h
static size_t count_lbraces(Token *tokens, size_t num_tokens) {
    size_t ret;
    for (size_t i = 0; i < num_tokens; i++) {
        if (tokens[i].ttype == LParen) ret++;
    }
    return ret;
}

static ASTBranch *new_branch(ASTBranch branch) {
    ASTBranch *ret = (ASTBranch*) malloc(sizeof(branch));
    *ret = branch;
    return ret;
}

static bool token_in_brackets(size_t idx, Token *tokens, size_t num_tokens) {
    size_t depth = 0;
    size_t i = 0;
    while (i < num_tokens) {
        if (idx == i) return depth != 0;
        if (tokens[i].ttype == LParen)
            depth += 1;
        else if (tokens[i].ttype == RParen)
            depth -= 1;
        i++;
    }
    printf("idx > num_tokens in token_in_brackets()\n");
    exit(1);
}

static size_t find_max_priority_token_idx(Token *tokens, size_t num_tokens) {
    if (num_tokens == 0) {
        printf("find_max_priority_token_idx() gets num_tokens=0\n");
        exit(1);
    }
    size_t ret = 0;
    size_t max = 0;
    for (size_t i = 0; i < num_tokens; i++) {
        if (is_operation(tokens[i].ttype)) {
            if (tokens[i].ttype == Not) continue; // TODO: Cases to skip for more unary ops
            else if (token_in_brackets(i, tokens, num_tokens)) continue;
            uint64_t priority = get_priority(tokens[i].ttype);
            if (priority >= max) {
                max = priority;
                ret = i;
            }
        }
    }
    if (max == 0) {
        return find_max_priority_token_idx(&tokens[1], num_tokens - 1);
    }
    return ret;
}

#define PRINT_SPACES(x) \
    for (size_t i = 0; i < x; i++) { \
        printf(" "); \
    }

// Don't worry too much about this being recursive because it's only really used for debugging
void print_ast(ASTBranch *ast, size_t depth) {
    if (ast->type == Number) {
        PRINT_SPACES(depth); printf(" - Number: %llu\n", ast->number);
    } else if (ast->type == UnaryOp) {
        PRINT_SPACES(depth); printf(" - UnaryNode: - Op: %s, val:\n", ttype_as_str(ast->unaryop.op));
        print_ast(ast->unaryop.val, depth + 13);
    } else if (ast->type == BinOp) {
        PRINT_SPACES(depth); printf(" - BinOpNode: - Op: %s, vals:\n", ttype_as_str(ast->binop.op.ttype));
        print_ast(ast->binop.left_val, depth + 13);
        print_ast(ast->binop.right_val, depth + 13);
    } else {
        PRINT_SPACES(depth); printf(" - Unknown AST branch type\n");
    }
}

ASTBranch *parse_expression(Token *tokens, size_t num_tokens) {
    if (tokens[num_tokens - 1].ttype == NewLine) num_tokens--;
    if (tokens[0].ttype == LParen && tokens[num_tokens - 1].ttype == RParen) {
        tokens = &tokens[1];
        num_tokens -= 2;
    }
    size_t num_lbraces = count_lbraces(tokens, num_tokens);
    // FIXME: It currently will break if there are any more operations on the right of braces which have
    // a unary operator
    if (num_tokens == 2 || num_tokens > 2 && (tokens[1].ttype == LParen && num_lbraces == 1)) {
        // FIXME: Make this not so horribly bad and stupid
        if (tokens[0].ttype == Not) return new_branch((ASTBranch) {.type=UnaryOp,.unaryop={.op=Not, .val=parse_expression(&tokens[1], num_tokens - 1)}});
        else {
            printf("Unknown unary expression in operation: %s\n", ttype_as_str(tokens[0].ttype)); // TODO: Proper error handler
            exit(1);
        }
    }
    bool tok0_is_literal = is_literal(tokens[0].ttype);
    if (num_tokens == 1 && (tok0_is_literal || tokens[0].ttype == Varname || tokens[0].ttype == StrLit)) {
        if (tokens[0].ttype == UnsignedInt) return new_branch((ASTBranch) {.type=Number,.number = tokens[0].numval});
        if (tokens[0].ttype == SignedInt) return new_branch((ASTBranch) {.type=Number,.number = tokens[0].numval});
        if (tokens[0].ttype == Float) return new_branch((ASTBranch) {.type=Number,.number = tokens[0].numval});
        if (tokens[0].ttype == Varname) return new_branch((ASTBranch) {.type=Var,.var = tokens[0].strval});
        if (tokens[0].ttype == StrLit) return new_branch((ASTBranch) {.type=Str,.var = tokens[0].strval});
        else {
            printf("Immediate value not yet supported (expression parser)\n"); // TODO: Proper error handler
            exit(1);
        }
    }
    if (num_tokens >= 3 && tok0_is_literal && tokens[0].ttype == LParen && tokens[num_tokens - 1].ttype == RParen) {
        printf("TODO: Function calls aren't supported yet.");
        exit(1);
    }
    size_t max_priority_idx = find_max_priority_token_idx(tokens, num_tokens);
    Token max_priority_tok = tokens[max_priority_idx];
    // From here, it's (seemingly) a BinOp
    ASTBranch *left_branch  = parse_expression(tokens, max_priority_idx);
    ASTBranch *right_branch = parse_expression(&tokens[max_priority_idx + 1], num_tokens - max_priority_idx - 1);
    return new_branch((ASTBranch) {
        .type=BinOp,
        .binop = (BinOpNode) {
            .left_val = left_branch,
            .op = max_priority_tok.ttype,
            .right_val = right_branch,
        }
    });
}
