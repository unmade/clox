#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"
#include "interpreter.h"
#include "loxobj.h"
#include "scanner.h"

static LoxObj *eval_unary(const Expr *expr);
static LoxObj *eval_literal(const Expr *expr);
static LoxObj *eval_binary(const Expr *expr);
static char *joinstr(const char *s1, const char *s2);


LoxObj *eval(const Expr *expr)
{
    switch(expr->type) {
        case EXPR_GROUPING:
            return eval(expr->grouping);
        case EXPR_UNARY:
            return eval_unary(expr);
        case EXPR_LITERAL:
            return eval_literal(expr);
        case EXPR_BINARY:
            return eval_binary(expr);
        default:
            return NULL;
    }
}


static LoxObj *eval_unary(const Expr *expr) 
{
    LoxObj *right, *obj;

    if ((right = eval(expr->unary.right)) == NULL)
        return NULL;

    switch (expr->unary.op->type) {
        case TOKEN_BANG:
            obj = new_bool_obj(!is_obj_truthy(right));
            free_obj(right);
            return obj;
        case TOKEN_MINUS:
            if (right->type != LOX_OBJ_NUMBER)
                return NULL;
            right->fval *= -1;
            return right;
        default:
            return NULL;
    }
}


static LoxObj *eval_literal(const Expr *expr)
{
    switch (expr->literal->type) {
        case TOKEN_NUMBER:
            return new_num_obj(atof(expr->literal->lexeme));
        case TOKEN_STRING:
            return new_str_obj(strdup(expr->literal->lexeme));
        case TOKEN_FALSE:
            return new_bool_obj(false);
        case TOKEN_TRUE:
            return new_bool_obj(true);
        case TOKEN_NIL:
            return new_nil_obj();
        default:
            return NULL;
    }
}


static LoxObj *eval_binary(const Expr *expr)
{
    LoxObj *left, *right;
    LoxObj *obj = NULL;

    if ((left = eval(expr->binary.left)) == NULL)
        return NULL;

    if ((right = eval(expr->binary.right)) == NULL)
        return NULL;

    switch (expr->binary.op->type) {
        case TOKEN_MINUS:
            if (left->type == LOX_OBJ_NUMBER && right->type == LOX_OBJ_NUMBER)
                obj = new_num_obj(left->fval - right->fval);
            break;
        case TOKEN_SLASH:
            if (left->type == LOX_OBJ_NUMBER && right->type == LOX_OBJ_NUMBER)
                obj = new_num_obj(left->fval / right->fval);
            break;
        case TOKEN_STAR:
            if (left->type == LOX_OBJ_NUMBER && right->type == LOX_OBJ_NUMBER)
                obj = new_num_obj(left->fval * right->fval); 
            break;
        case TOKEN_PLUS:
            if (left->type == LOX_OBJ_NUMBER && right->type == LOX_OBJ_NUMBER)
                obj = new_num_obj(left->fval + right->fval);
            else if (left->type == LOX_OBJ_STRING && right->type == LOX_OBJ_STRING)
                obj = new_str_obj(joinstr(left->sval, right->sval));
            break;
        default:
            break;
    }

    free_obj(left);
    free_obj(right);

    return obj;
}


static char *joinstr(const char *s1, const char *s2)
{
    int len1, len2;
    char *s;

    len1 = strlen(s1);
    len2 = strlen(s2);

    s = (char *) malloc((len1 + len2) * sizeof(char));
    strcat(s, s1);
    strcat(s + len1, s2);

    return s;
}
