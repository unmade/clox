#ifndef clox_stmt_h
#define clox_stmt_h

#include "expr.h"

struct loxenv;

enum StmtType {
    STMT_BLOCK = 0,
    STMT_FUN,
    STMT_EXPR,
    STMT_IF,
    STMT_PRINT,
    STMT_RETURN,
    STMT_VAR,
    STMT_WHILE,
};


typedef struct stmt {
    enum StmtType type;
    union {
        Expr *expr;
        struct { size_t n; struct stmt **stmts; } block;
        struct { char *name; size_t n; Token **params; struct stmt *body; } fun;
        struct { Expr *cond; struct stmt *conseq; struct stmt *alt; } ifelse;
        struct { char *name; Expr *expr; } var;
        struct { Expr *cond; struct stmt *body; } whileloop;
    };
} Stmt;

Stmt *new_block_stmt(size_t n, Stmt **stmts);
Stmt *new_fun_stmt(char *name, size_t n, Token **params, Stmt *body);
Stmt *new_expr_stmt(Expr *expr);
Stmt *new_if_stmt(Expr *cond, Stmt *conseq, Stmt *alt);
Stmt *new_print_stmt(Expr *expr);
Stmt *new_return_stmt(Expr *expr);
Stmt *new_var_stmt(char *name, Expr *expr);
Stmt *new_while_stmt(Expr *cond, Stmt *body);

void free_stmt(Stmt *stmt);

#endif
