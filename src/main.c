#include <stdio.h>
#include <lexer.h>
#include <expression.h>

int main() {
    char *test = "$5 + ($3 * $4)\n";
    Token *tokens;
    size_t num_tokens = lex(test, &tokens);
    ASTBranch *ast = parse_expression(tokens, num_tokens);
    printf("Parsing complete, AST:\n");
    print_ast(ast, 0);
}
