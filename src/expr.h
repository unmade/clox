#ifndef clox_expr_h
#define clox_expr_h

#include "scanner.h"

enum ExprType {
    EXPR_ASSIGN = 0,
    EXPR_BINARY,
    EXPR_CALL,
    EXPR_GET,
    EXPR_GROUPING,
    EXPR_LITERAL,
    EXPR_SET,
    EXPR_SUPER,
    EXPR_THIS,
    EXPR_UNARY,
    EXPR_VAR,
};


typedef struct expr {
    enum ExprType type;
    union {
        struct { Token *name; struct expr *value; } assign;
        struct { struct expr *left; Token *op; struct expr *right; } binary;
        struct { struct expr *callee; Token *paren; size_t argc; struct expr **args; } call;
        struct { Token *name; struct expr *object; } get;
        struct expr *grouping;
        Token *literal;
        Token *keyword;
        struct { Token *name; struct expr *object; struct expr *value; } set;
        struct { Token *keyword; Token *method; } super;
        struct { Token *op; struct expr *right; } unary;
        Token *varname;
    };
} Expr;


Expr *new_assign_expr(Token *name, Expr *value);
Expr *new_binary_expr(Expr *left, Token *op, Expr *right);
Expr *new_call_expr(Expr *callee, Token *paren, size_t argc, Expr **args);
Expr *new_get_expr(Token *name, Expr *object);
Expr *new_grouping_expr(Expr *expr);
Expr *new_literal_expr(Token *literal);
Expr *new_set_expr(Token *name, Expr *object, Expr *value);
Expr *new_super_expr(Token *keyword, Token *method);
Expr *new_this_expr(Token *keyword);
Expr *new_unary_expr(Token *op, Expr *right);
Expr *new_var_expr(Token *name);

void free_expr(Expr *expr);


void print_expr(const Expr *expr);
char *str_expr(const Expr *expr);

#endif
