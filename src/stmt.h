#ifndef clox_stmt_h
#define clox_stmt_h

#include "expr.h"

enum StmtType {
    STMT_BLOCK = 0,
    STMT_EXPR,
    STMT_PRINT,
    STMT_VAR,
};


typedef struct stmt {
    enum StmtType type;
    union {
        Expr *expr;
        struct { size_t n; struct stmt **stmts; } block;
        struct { char *name; Expr *expr; } var;
    };
} Stmt;

Stmt *new_block_stmt(size_t n, Stmt **stmts);
Stmt *new_expr_stmt(Expr *expr);
Stmt *new_print_stmt(Expr *expr);
Stmt *new_var_stmt(char *name, Expr *expr);
void free_stmt(Stmt *stmt);

#endif
