#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "scanner.h"
#include "expr.h"

static int _str_expr(char *s, unsigned len, size_t *maxlen, const Expr *expr);
static int join_expr(
    char *s,
    unsigned len,
    size_t *maxlen,
    const char *name,
    unsigned exprc,
    ...
);


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


void print_expr(const Expr *expr)
{
    char *s;

    s = str_expr(expr);
    printf("%s\n", s);
    free(s);
}


char *str_expr(const Expr *expr)
{
    unsigned len = 0;
    size_t maxlen = 8;
    char *s = (char *) malloc(maxlen * sizeof(char));

    s[0] = '\0';
    _str_expr(s, len, &maxlen, expr);
    
    return s;
}


static int _str_expr(char *s, unsigned len, size_t *maxlen, const Expr *expr)
{
    switch(expr->type) {
        case EXPR_BINARY:
            return join_expr(s, len, maxlen, expr->binary.op->lexeme,
                            2, expr->binary.left, expr->binary.right);
        case EXPR_GROUPING:
            return join_expr(s, len, maxlen, "group",
                            1, expr->grouping);
        case EXPR_LITERAL:
            return join_expr(s, len, maxlen, expr->literal->lexeme, 0);
        case EXPR_UNARY:
            return join_expr(s, len, maxlen, expr->unary.op->lexeme,
                            1, expr->unary.right);
        default:
            return len;
    }
}


static int join_expr(
    char *s, 
    unsigned len,
    size_t *maxlen,
    const char *name,
    unsigned exprc,
    ...
) {
    Expr *expr;
    va_list exprs;

    unsigned nlen = (len + strlen(name) + 3);
    bool need_paren = (exprc > 0);

    if (*maxlen < nlen)
        while (*maxlen < nlen)
            s = (char *) realloc(s, (*maxlen *= 2) * sizeof(char));

    if (need_paren)
        strcat(s + len++, "(");

    while ((s[len++] = *name++))
        ;
    --len;

    va_start(exprs, exprc);

    while (exprc--) {
        strcat(s + len++, " ");
        expr = va_arg(exprs, Expr *);
        len = _str_expr(s, len, maxlen, expr);
    }

    va_end(exprs);

    if (need_paren)
        strcat(s + len++, ")");

    return len;
}
