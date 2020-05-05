#ifndef clox_expr_h
#define clox_expr_h

#include "scanner.h"

enum ExprType {
    EXPR_BINARY = 0,
    EXPR_GROUPING,
    EXPR_LITERAL,
    EXPR_UNARY,
};


typedef struct expr {
    enum ExprType type;
    union {
        struct { struct expr *left; Token *op; struct expr *right; } binary;
        struct expr *grouping;
        Token *literal;
        struct { Token *op; struct expr *right; } unary;
    };
} Expr;


Expr *new_binary_expr(Expr *left, Token *op, Expr *right);
Expr *new_grouping_expr(Expr *expr);
Expr *new_literal_expr(Token *literal);
Expr *new_unary_expr(Token *op, Expr *right);
char *str_expr(char *str, Expr *expr);

#endif
