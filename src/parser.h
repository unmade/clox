#ifndef clox_parser_h
#define clox_parser_h

#include "scanner.h"
#include "stmt.h"

Stmt **parse(Token *tokens);
void free_stmts(Stmt **stmts);

#endif
