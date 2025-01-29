#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <lexer.h>
#include <parser.h>

int main() {
    Token *tokens;
    FunctionSignature *functions;
    FILE *f = fopen("test.fat", "r");
    if (!f) {
        printf("Couldn't open test.fat!\n");
        return 1;
    }
    fseek(f, 0L, SEEK_END);
    size_t flen = ftell(f);
    fseek(f, 0L, SEEK_SET);
    char *file_contents = (char*) malloc(flen + 1);
    size_t read_status;
    if ((read_status = fread(file_contents, flen, 1, f)) != 1) {
        printf("Couldn't read test.fat, status = %llu\n", read_status);
        return 1;
    }
    fclose(f);
    size_t num_tokens = lex(file_contents, &tokens);
    printf("Lexing complete.\n");
    size_t num_statements = parse_program(tokens, num_tokens, &functions);
    printf("Parsing complete.\n");
}
