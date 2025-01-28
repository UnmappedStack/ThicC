#include <stdio.h>
#include <lexer.h>
#include <expression.h>
#include <statement.h>

int main() {
    char *test = "functionName($5 + !($3 * $4), 8)\n";
    Token *tokens;
    size_t num_tokens = lex(test, &tokens);
    parse_statement(tokens, num_tokens);
}
