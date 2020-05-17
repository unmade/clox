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


Stmt *new_fun_stmt(char *name, size_t n, Token **params, Stmt *body)
{
    Stmt *stmt = (Stmt *) malloc(sizeof(Stmt));

    stmt->type = STMT_FUN;
    stmt->fun.name = name;
    stmt->fun.n = n;
    stmt->fun.params = params;
    stmt->fun.body = body;

    return stmt;
}


Stmt *new_if_stmt(Expr *cond, Stmt *conseq, Stmt *alt)
{
    Stmt *stmt = (Stmt *) malloc(sizeof(Stmt));

    stmt->type = STMT_IF;
    stmt->ifelse.cond = cond;
    stmt->ifelse.conseq = conseq;
    stmt->ifelse.alt = alt;

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


Stmt *new_while_stmt(Expr *cond, Stmt *body)
{
    Stmt *stmt = (Stmt *) malloc(sizeof(Stmt));

    stmt->type = STMT_WHILE;
    stmt->whileloop.cond = cond;
    stmt->whileloop.body = body;

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
        case STMT_FUN:
            free(stmt->fun.name);
            free_stmt(stmt->fun.body);
            break;
        case STMT_IF:
            free_expr(stmt->ifelse.cond);
            free_stmt(stmt->ifelse.conseq);
            if (stmt->ifelse.alt != NULL)
                free_stmt(stmt->ifelse.alt);
            break;
        case STMT_PRINT:
            free_expr(stmt->expr);
            break;
        case STMT_VAR:
            free(stmt->var.name);
            if (stmt->var.expr != NULL)
                free_expr(stmt->var.expr);
            break;
        case STMT_WHILE:
            free_expr(stmt->whileloop.cond);
            free_stmt(stmt->whileloop.body);
            break;
        default:
            break;
    }

    free(stmt);
}
