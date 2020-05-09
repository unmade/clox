#include <stdlib.h>
#include <string.h>

#include "expr.h"
#include "interpreter.h"
#include "scanner.h"

int is_truthy(ExprResult *res) 
{
    switch (res->type) {
        case RESULT_NUMBER:
        case RESULT_STRING:
        case RESULT_NIL:
            return 0;
        case RESULT_BOOL:
            return (res->ival != 0);
        default:
            return 0;
    }
}


static ExprResult *new_unary(Expr *expr) 
{
    int b;
    ExprResult *right;

    if ((right = eval(expr->unary.right)) == NULL)
        return NULL;

    // don't forget to clean up 'right'
    // since values are overridden
    // we need to clean old values

    switch (expr->unary.op->type) {
        case TOKEN_BANG:
            if ((b = !is_truthy(right))) {
                right->type = RESULT_BOOL;
                right->ival = b; 
            } 
            return right;
        case TOKEN_MINUS:
            if (right->type != RESULT_NUMBER)
                return NULL;
            right->fval *= -1;
            return right;
        default:
            return NULL;
    }
}


static ExprResult *new_literal(Expr *expr)
{
    ExprResult *res;

    res = (ExprResult *) malloc(sizeof(ExprResult));

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
            res->ival = 0;
            return res;
        case TOKEN_TRUE:
            res->type = RESULT_BOOL;
            res->ival = 1;
            return res;
        case TOKEN_NIL:
            res->type = RESULT_NIL;
            return res;
        default:
            break;
    }

    free(res);
    return NULL;
}


static ExprResult *eval_binary(Expr *expr)
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


ExprResult *eval(Expr *expr)
{
    switch(expr->type) {
        case EXPR_GROUPING:
            return eval(expr->grouping);
        case EXPR_UNARY:
            return new_unary(expr);
        case EXPR_LITERAL:
            return new_literal(expr);
        case EXPR_BINARY:
            return eval_binary(expr);
        default:
            break;
    }

    return NULL;
}


char *str_expr_res(const ExprResult *res)
{
    char *s;
    size_t flen;

    switch (res->type) {
        case RESULT_BOOL:
            return strdup((res->ival) ? "true" : "false");
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
