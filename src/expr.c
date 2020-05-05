#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "scanner.h"
#include "expr.h"

static char *paren(char *str, char *name, int exprc, ...);


Expr *new_binary_expr(Expr *left, Token *op, Expr *right)
{
    Expr *expr = (Expr *) malloc(sizeof(Expr));

    expr->type = EXPR_BINARY;
    expr->binary.left = left;
    expr->binary.op = op;
    expr->binary.right = right;

    return expr;
}


Expr *new_grouping_expr(Expr *group)
{
    Expr *expr = (Expr *) malloc(sizeof(Expr));

    expr->type = EXPR_GROUPING;
    expr->grouping = group;

    return expr;
}


Expr *new_literal_expr(Token *literal)
{
    Expr *expr = (Expr *) malloc(sizeof(Expr));

    expr->type = EXPR_LITERAL;
    expr->literal = literal;

    return expr;
}


Expr *new_unary_expr(Token *op, Expr *right)
{
    Expr *expr = (Expr *) malloc(sizeof(Expr));

    expr->type = EXPR_UNARY;
    expr->unary.op = op;
    expr->unary.right = right;

    return expr;
}


char *str_expr(char *str, Expr *expr)
{
    char *s = str; 

    switch(expr->type) {
        case EXPR_BINARY:
            paren(str, expr->binary.op->repr, 2, expr->binary.left, expr->binary.right);
            break;
        case EXPR_GROUPING:
            paren(str, "group", 1, expr->grouping);
            break;
        case EXPR_LITERAL:
            strcat(s, expr->literal->repr);
            break;
        case EXPR_UNARY:
            paren(str, expr->unary.op->repr, 1, expr->unary.right);
            break;
        default:
            break;
    }

    return str;
}


static char *paren(char *str, char *name, int exprc, ...)
{
    Expr *expr;
    va_list exprs;

    strcat(str, "(");
    strcat(str, name);

    va_start(exprs, exprc);

    while (exprc--) {
        strcat(str, " ");
        expr = va_arg(exprs, Expr *);
        str_expr(str, expr);
    }

    va_end(exprs);

    strcat(str, ")");

    return str;
}
