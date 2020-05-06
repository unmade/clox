#include "expr.h"
#include "scanner.h"

struct tokenlist {
    Token *curr;
};

static Expr *expression(struct tokenlist *tlist);
static Expr *equality(struct tokenlist *tlist);
static Expr *comparison(struct tokenlist *tlist);
static Expr *addition(struct tokenlist *tlist);
static Expr *multiplication(struct tokenlist *tlist);
static Expr *unary(struct tokenlist *tlist);
static Expr *primary(struct tokenlist *tlist);


static Token *get_token(struct tokenlist *tlist)
{
    Token *prev = tlist->curr;
    tlist->curr = tlist->curr->next;
    return prev;
}


static Token *peek_token(struct tokenlist *tlist)
{
    return tlist->curr;
}


Expr *parse(Token *tokens)
{
    struct tokenlist tlist;

    if (tokens == NULL)
        return NULL;

    tlist.curr = tokens;

    return expression(&tlist);    
}


static Expr *expression(struct tokenlist *tlist)
{
    return equality(tlist);
}


static Expr *equality(struct tokenlist *tlist)
{
    Expr *expr, *right;
    Token *token;

    if ((expr = comparison(tlist)) == NULL)
        return NULL;

    while ((token = peek_token(tlist)) != NULL) {
        if (token->type == TOKEN_ERROR)
            return NULL;

        if (token->type == TOKEN_BANG_EQUAL || token->type == TOKEN_EQUAL_EQUAL) {
            get_token(tlist);
            if ((right = comparison(tlist)) == NULL)
                return NULL;
            expr = new_binary_expr(expr, token, right);
            continue;
        }

        break;
    }
        
    return expr;
}


static Expr *comparison(struct tokenlist *tlist)
{
    Expr *expr, *right;
    Token *token;

    if ((expr = addition(tlist)) == NULL)
        return NULL;

    while ((token = peek_token(tlist)) != NULL) {
        if (token->type == TOKEN_ERROR)
            return NULL;

        if (token->type == TOKEN_GREATER || token->type == TOKEN_GREATER_EQUAL \
            || token->type == TOKEN_LESS || token->type == TOKEN_LESS_EQUAL) {
            get_token(tlist);
            if ((right = addition(tlist)) == NULL)
                return NULL;
            expr = new_binary_expr(expr, token, right);
            continue;
        }

        break;
    }
 
    return expr;
}


static Expr *addition(struct tokenlist *tlist)
{

    Expr *expr, *right;
    Token *token;

    if ((expr = multiplication(tlist)) == NULL)
        return NULL;

    while ((token = peek_token(tlist)) != NULL) {
        if (token->type == TOKEN_ERROR)
            return NULL;

        if (token->type == TOKEN_MINUS || token->type == TOKEN_PLUS) {
            get_token(tlist);
            if ((right = multiplication(tlist)) == NULL)
                return NULL;
            expr = new_binary_expr(expr, token, right);
            continue;
        }

        break;
    }
        
    return expr;
}


static Expr *multiplication(struct tokenlist *tlist)
{

    Expr *expr, *right;
    Token *token;

    if ((expr = unary(tlist)) == NULL)
        return NULL;

    while ((token = peek_token(tlist)) != NULL) {
        if (token->type == TOKEN_ERROR)
            return NULL;

        if (token->type == TOKEN_SLASH || token->type == TOKEN_STAR) {
            get_token(tlist);
            if ((right = unary(tlist)) == NULL)
                return NULL;
            expr = new_binary_expr(expr, token, right);
            continue;
        }

        break;
    }
 
    return expr;
}


static Expr *unary(struct tokenlist *tlist)
{
    Expr *expr;
    Token *token;

    if ((token = peek_token(tlist)) != NULL) {
        if (token->type == TOKEN_ERROR)
            return NULL;

        if (token->type == TOKEN_BANG || token->type == TOKEN_MINUS) {
            get_token(tlist);
            if ((expr = unary(tlist)) == NULL)
                return NULL;
            return new_unary_expr(token, expr);
        }
    } 

    return primary(tlist);
}


static Expr *primary(struct tokenlist *tlist)
{
    Expr *expr;
    Token *token;

    if ((token = get_token(tlist)) != NULL) {
        if (token->type == TOKEN_ERROR)
            return NULL;

        if (token->type == TOKEN_LEFT_PAREN) {
            expr = expression(tlist);
            token = get_token(tlist);
            if (token == NULL || (token != NULL && token->type != TOKEN_RIGHT_PAREN))
                return NULL;
            return expr;
        }

        return new_literal_expr(token);
    }

    return NULL;
}
