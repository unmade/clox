#ifndef clox_interpreter_h
#define clox_interpreter_h

#include "expr.h"
#include "loxobj.h"
#include "stmt.h"

int interpret(Stmt **stmt);

#endif
