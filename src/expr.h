#ifndef clox_expr_h
#define clox_expr_h

#include "scanner.h"

enum ExprType {
    EXPR_ASSIGN = 0,
    EXPR_BINARY,
    EXPR_GROUPING,
    EXPR_LITERAL,
    EXPR_UNARY,
    EXPR_VAR,
};


typedef struct expr {
    enum ExprType type;
    union {
        struct { Token *name; struct expr *value; } assign;
        struct { struct expr *left; Token *op; struct expr *right; } binary;
        struct expr *grouping;
        Token *literal;
        struct { Token *op; struct expr *right; } unary;
        Token *varname;
    };
} Expr;


Expr *new_assign_expr(Token *name, Expr *value);
Expr *new_binary_expr(Expr *left, Token *op, Expr *right);
Expr *new_grouping_expr(Expr *expr);
Expr *new_literal_expr(Token *literal);
Expr *new_unary_expr(Token *op, Expr *right);
Expr *new_var_expr(Token *name);
void print_expr(const Expr *expr);
char *str_expr(const Expr *expr);

#endif
