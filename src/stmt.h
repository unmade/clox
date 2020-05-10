#ifndef clox_stmt_h
#define clox_stmt_h

#include "expr.h"

enum StmtType {
    STMT_EXPR = 0,
    STMT_PRINT,
};


typedef struct stmt {
    enum StmtType type;
    union {
        Expr *expr;
    };
} Stmt;

Stmt *new_expr_stmt(Expr *expr);
Stmt *new_print_stmt(Expr *expr);

#endif
