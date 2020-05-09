#ifndef clox_interpreter_h
#define clox_interpreter_h

#include <stdbool.h>

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
        bool bval;
        float fval;
        char *sval;
    };
} ExprResult;


ExprResult *eval(const Expr *expr);
void free_expr_res(ExprResult *res);
char *str_expr_res(const ExprResult *res);
void print_expr_res(const ExprResult *res);

#endif
