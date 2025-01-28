#pragma once
#include <lexer.h>
#include <stddef.h>
#include <ast.h>

ASTBranch *parse_expression(Token *tokens, size_t num_tokens);
