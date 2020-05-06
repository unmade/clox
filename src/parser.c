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
    Expr *expr = comparison(tlist);
    Token *token;

    while (
        (token = peek_token(tlist)) != NULL && (token->type == TOKEN_BANG_EQUAL || token->type == TOKEN_EQUAL_EQUAL)
    ) {
        get_token(tlist);
        Expr *right = comparison(tlist);
        expr = new_binary_expr(expr, token, right);
    } 
        
    return expr;
}


static Expr *comparison(struct tokenlist *tlist)
{
    Expr *expr = addition(tlist);
    Token *token;

    while (
        (token = peek_token(tlist)) != NULL && (token->type == TOKEN_GREATER || token->type == TOKEN_GREATER_EQUAL || token->type == TOKEN_LESS || token->type == TOKEN_LESS_EQUAL)
    ) {
        get_token(tlist);
        Expr *right = addition(tlist);
        expr = new_binary_expr(expr, token, right);
    } 
        
    return expr;

}


static Expr *addition(struct tokenlist *tlist)
{

    Expr *expr = multiplication(tlist);
    Token *token;

    while (
        (token = peek_token(tlist)) != NULL && (token->type == TOKEN_MINUS || token->type == TOKEN_PLUS)
    ) {
        get_token(tlist);
        Expr *right = multiplication(tlist);
        expr = new_binary_expr(expr, token, right);
    } 
        
    return expr;
}


static Expr *multiplication(struct tokenlist *tlist)
{

    Expr *expr = unary(tlist);
    Token *token;

    while (
        (token = peek_token(tlist)) != NULL && (token->type == TOKEN_SLASH || token->type == TOKEN_STAR)
    ) {
        get_token(tlist);
        Expr *right = unary(tlist);
        expr = new_binary_expr(expr, token, right);
    } 
        
    return expr;
}


static Expr *unary(struct tokenlist *tlist)
{
   Expr *expr; 
   Token *token;

    if (
        (token = peek_token(tlist)) != NULL && (token->type == TOKEN_BANG || token->type == TOKEN_MINUS)
    ) {
        get_token(tlist);
        expr = unary(tlist);
        return new_unary_expr(token, expr);
    }
    return primary(tlist);
}


static Expr *primary(struct tokenlist *tlist)
{
    Expr *expr;
    Token *token;

    if ((token = get_token(tlist)) != NULL && token->type == TOKEN_LEFT_PAREN) {
        expr = expression(tlist);
        if (get_token(tlist)->type != TOKEN_RIGHT_PAREN)
            return NULL;
        return expr;
    }

    return new_literal_expr(token);
}
