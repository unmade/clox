#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"
#include "interpreter.h"
#include "scanner.h"

static bool is_truthy(const ExprResult *res);
static ExprResult *eval_unary(const Expr *expr);
static ExprResult *eval_literal(const Expr *expr);
static ExprResult *eval_binary(const Expr *expr);


ExprResult *eval(const Expr *expr)
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


void free_expr_res(ExprResult *res)
{
    switch (res->type) {
        case RESULT_NUMBER:
        case RESULT_BOOL:
        case RESULT_NIL:
            free(res);
            break;
        case RESULT_STRING:
            free(res->sval);
            break;
        default:
            break;
    }
}


static bool is_truthy(const ExprResult *res) 
{
    switch (res->type) {
        case RESULT_BOOL:
            return (res->bval == true);
        default:
            return false;
    }
}


static ExprResult *eval_unary(const Expr *expr) 
{
    ExprResult *right, *res;

    if ((right = eval(expr->unary.right)) == NULL)
        return NULL;

    switch (expr->unary.op->type) {
        case TOKEN_BANG:
            res = (ExprResult *) malloc(sizeof(ExprResult));
            res->type = RESULT_BOOL;
            res->bval = !is_truthy(right);
            free_expr_res(right);
            return res;
        case TOKEN_MINUS:
            if (right->type != RESULT_NUMBER)
                return NULL;
            right->fval *= -1;
            return right;
        default:
            return NULL;
    }
}


static ExprResult *eval_literal(const Expr *expr)
{
    ExprResult *res = (ExprResult *) malloc(sizeof(ExprResult));

    switch (expr->literal->type) {
        case TOKEN_NUMBER:
            res->type = RESULT_NUMBER;
            res->fval = atof(expr->literal->lexeme);
            return res;
        case TOKEN_STRING:
            res->type = RESULT_STRING;
            res->sval = strdup(expr->literal->lexeme);
            return res;
        case TOKEN_FALSE:
            res->type = RESULT_BOOL;
            res->bval = false;
            return res;
        case TOKEN_TRUE:
            res->type = RESULT_BOOL;
            res->bval = true;
            return res;
        case TOKEN_NIL:
            res->type = RESULT_NIL;
            return res;
        default:
            free(res);
            return NULL;
    }
}


static ExprResult *eval_binary(const Expr *expr)
{
    // there must be a better way!

    ExprResult *left, *right, *res;

    if ((left = eval(expr->binary.left)) == NULL)
        return NULL;

    if ((right = eval(expr->binary.right)) == NULL)
        return NULL;

    res = (ExprResult *) malloc(sizeof(ExprResult));

    switch (expr->binary.op->type) {
        case TOKEN_MINUS:
            if (left->type == RESULT_NUMBER && right->type == RESULT_NUMBER) {
                res->type = RESULT_NUMBER;
                res->fval = left->fval - right->fval;
                return res;
            }
            break;
        case TOKEN_SLASH:
            if (left->type == RESULT_NUMBER && right->type == RESULT_NUMBER) {
                res->type = RESULT_NUMBER;
                res->fval = left->fval / right->fval;
                return res;
            }
            break;
        case TOKEN_STAR:
            if (left->type == RESULT_NUMBER && right->type == RESULT_NUMBER) {
                res->type = RESULT_NUMBER;
                res->fval = left->fval * right->fval;
                return res;
            }
            break;
        case TOKEN_PLUS:
            if (left->type == RESULT_NUMBER && right->type == RESULT_NUMBER) {
                res->type = RESULT_NUMBER;
                res->fval = left->fval + right->fval;
                return res;
            } else if (left->type == RESULT_STRING && right->type == RESULT_STRING) {
                res->type = RESULT_STRING;
                res->sval = (char *) malloc((strlen(left->sval) + strlen(right->sval)) * sizeof(char));
                strcat(res->sval, left->sval);
                strcat(res->sval, right->sval);
                return res; 
            }
            break;
        default:
            break;
    }

    free(res);
    return NULL;
}


char *str_expr_res(const ExprResult *res)
{
    char *s;
    size_t flen;

    switch (res->type) {
        case RESULT_BOOL:
            return strdup((res->bval) ? "true" : "false");
        case RESULT_NUMBER:
            flen = snprintf(NULL, 0, "%f", res->fval);
            s = (char *) malloc(flen * sizeof(char));
            sprintf(s, "%f", res->fval);
            return s;
        case RESULT_STRING:
            return strdup(res->sval);
        case RESULT_NIL:
            return strdup("nil");
        default:
            return NULL;
    }
}


void print_expr_res(const ExprResult *res)
{
    char *s;

    s = str_expr_res(res);
    printf("%s\n", s);
    free(s);
}
