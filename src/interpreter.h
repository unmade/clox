#ifndef clox_interpreter_h
#define clox_interpreter_h

#include "expr.h"

enum ExprResultType {
    RESULT_NUMBER = 0,
    RESULT_STRING,
    RESULT_BOOL,
    RESULT_NIL,
};


typedef struct {
    enum ExprResultType type;
    union {
        int ival;
        float fval;
        char *sval;
    };
} ExprResult;


ExprResult *eval(Expr *expr);
char *str_expr_res(const ExprResult *res);
void print_expr_res(const ExprResult *res);

#endif
