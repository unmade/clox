#include <stdlib.h>

#include "expr.h"
#include "stmt.h"


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
