#include <stdio.h>
#include <lexer.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define is_keyword_char(ch, is_first) (ch == '_' || isalpha(ch) || (isdigit(ch) && !is_first))

#define push_token(vec, size, val) \
    do { \
        vec[size] = val; \
        size++; \
        vec = realloc(vec, (size + 1) * sizeof(Token)); \
    } while (0)

bool is_type(TokenType tok) {
    // TODO: More types. This looks stupid cos of the lack of types (yes I know) but it'll be less dumb
    // once there are more types
    if (tok == UQword) return true;
    else return false;
}

// Returns number of tokens. Puts into **tokbuf the pointer to a list of tokens lexed.
size_t lex(char *txt, Token **tokbuf) {
    size_t len = strlen(txt);
    char *original = txt;
    txt = (char*) malloc(len + 1);
    memcpy(txt, original, len + 1);
    Token *tokens = (Token*) malloc(sizeof(Token));
    size_t num_tok = 0;
    while (*txt) {
        if (*txt == '\n') push_token(tokens, num_tok, (Token) {.ttype=NewLine});
        else if (*txt == ' ') {}
        else if (*txt == ':') push_token(tokens, num_tok, (Token) {.ttype=Colon});
        else if (*txt == ',') push_token(tokens, num_tok, (Token) {.ttype=Comma});
        else if (*txt == '=') push_token(tokens, num_tok, (Token) {.ttype=Equ});
        else if (*txt == '+') push_token(tokens, num_tok, (Token) {.ttype=Add});
        else if (*txt == '-') push_token(tokens, num_tok, (Token) {.ttype=Sub});
        else if (*txt == '/') push_token(tokens, num_tok, (Token) {.ttype=Div});
        else if (*txt == '*') push_token(tokens, num_tok, (Token) {.ttype=Mul});
        else if (*txt == '(') push_token(tokens, num_tok, (Token) {.ttype=LParen});
        else if (*txt == ')') push_token(tokens, num_tok, (Token) {.ttype=RParen});
        else if (*txt == '[') push_token(tokens, num_tok, (Token) {.ttype=LBracket});
        else if (*txt == ']') push_token(tokens, num_tok, (Token) {.ttype=RBracket});
        else if (*txt == '{') push_token(tokens, num_tok, (Token) {.ttype=LBrace});
        else if (*txt == '}') push_token(tokens, num_tok, (Token) {.ttype=RBrace});
        else if (*txt == '!') push_token(tokens, num_tok, (Token) {.ttype=Not});
        else if (*txt == '$') {
            size_t i = 0;
            for (; txt[i] >= '0' && txt[i] <= '9'; i++);
            char temp = txt[i];
            txt[i] = 0;
            uint64_t num = strtoll(txt + 1, NULL, 10);
            txt[i] = temp;
            push_token(tokens, num_tok, ((Token) {.ttype=UnsignedInt, .numval=num}));
            txt += i + 1;
        } else if (is_keyword_char(*txt, true)) {
            size_t i = 0;
            for (; is_keyword_char(txt[i], !i); i++);
            char *buf = (char*) malloc(i + 1);
            memcpy(buf, txt, i);
            buf[i] = 0;
            if (!strcmp(buf, "uqword")) {
                free(buf);
                push_token(tokens, num_tok, ((Token) {.ttype=UQword}));
            } else if (!strcmp(buf, "return")) {
                free(buf);
                push_token(tokens, num_tok, ((Token) {.ttype=Return}));
            } else {
                push_token(tokens, num_tok, ((Token) {.ttype=Identifier, .strval=buf}));
            }
            txt += i - 1;
        } else if (*txt == '"') {
            size_t i = 0;
            for (; !(txt[i] == '"' && i); i++);
            char *buf = (char*) malloc(i + 1);
            memcpy(buf, txt, i);
            buf[i] = 0;
            push_token(tokens, num_tok, ((Token) {.ttype=StrLit, .strval=buf+1}));
            txt += i;
        } else if (*txt == '%') {
            txt++;
            size_t i = 0;
            for (; is_keyword_char(txt[i], !i); i++);
            char *buf = (char*) malloc(i + 1);
            memcpy(buf, txt, i);
            buf[i] = 0;
            push_token(tokens, num_tok, ((Token) {.ttype=Varname, .strval=buf}));
            txt += i - 1;
        }
        else {
            printf("Unknown token! Character = %c\n", *txt);
            exit(1);
        }
        txt++;
    }
    *tokbuf = tokens;
    return num_tok;
}
