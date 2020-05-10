#ifndef clox_parser_h
#define clox_parser_h

#include "scanner.h"
#include "stmt.h"

Stmt **parse(Token *tokens);

#endif
