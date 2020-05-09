#ifndef clox_interpreter_h
#define clox_interpreter_h

#include "expr.h"
#include "loxobj.h"


LoxObj *eval(const Expr *expr);

#endif
