#include <stdio.h>
#include <lexer.h>
#include <expression.h>
#include <statement.h>

int main() {
    char *test = "%var: uqword = $5 + !($3 * $4)\n";
    Token *tokens;
    size_t num_tokens = lex(test, &tokens);
    parse_statement(tokens, num_tokens);
}
