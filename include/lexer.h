#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    Colon, Equ, Add, Sub, Div, Mul, LParen, RParen, LBracket, RBracket, LBrace, RBrace, 
    UnsignedInt, SignedInt, Float, UQword, Identifier, Varname, NewLine, Not, Comma, Return, StrLit, 
} TokenType;

typedef struct {
    TokenType ttype;
    union {
        uint64_t numval;
        char    *strval;
    };
} Token;

size_t lex(char *txt, Token **tokbuf);
char *ttype_as_str(TokenType tok);
bool is_type(TokenType tok);

#define is_literal(tok) (tok == SignedInt || tok == UnsignedInt || tok == Float)
#define is_operation(tok) (tok == Add || tok == Sub || tok == Div || tok == Mul)
