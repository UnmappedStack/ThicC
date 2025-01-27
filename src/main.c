#include <stdio.h>
#include <lexer.h>

int main() {
    char *test = "%var: uqword = $5 + $3\n";
    lex(test);
}
