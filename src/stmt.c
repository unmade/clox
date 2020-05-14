#include <stdlib.h>

#include "expr.h"
#include "stmt.h"


Stmt *new_block_stmt(size_t n, Stmt **stmts)
{
    Stmt *stmt = (Stmt *) malloc(sizeof(Stmt));

    stmt->type = STMT_BLOCK;
    stmt->block.n = n;
    stmt->block.stmts = stmts;

    return stmt;
}


Stmt *new_expr_stmt(Expr *expr)
{
    Stmt *stmt = (Stmt *) malloc(sizeof(Stmt));

    stmt->type = STMT_EXPR;
    stmt->expr = expr;

    return stmt;
}


Stmt *new_print_stmt(Expr *expr)
{
    Stmt *stmt = (Stmt *) malloc(sizeof(Stmt));

    stmt->type = STMT_PRINT;
    stmt->expr = expr;

    return stmt;
}


Stmt *new_var_stmt(char *name, Expr *expr)
{
    Stmt *stmt = (Stmt *) malloc(sizeof(Stmt));

    stmt->type = STMT_VAR;
    stmt->var.name = name;
    stmt->var.expr = expr;

    return stmt;
}


void free_stmt(Stmt *stmt)
{
    unsigned i;

    switch (stmt->type) {
        case STMT_BLOCK:
            for (i = 0; i < stmt->block.n; i++)
                free_stmt(stmt->block.stmts[i]); 
            free(stmt->block.stmts);
            break;
        case STMT_EXPR:
            free_expr(stmt->expr);
            break;
        case STMT_PRINT:
            free_expr(stmt->expr);
            break;
        case STMT_VAR:
            free(stmt->var.name);
            free_expr(stmt->var.expr);
            break;
        default:
            break;
    }

    free(stmt);
}
