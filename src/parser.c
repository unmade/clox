#include <stdbool.h>
#include <stdlib.h>

#include "expr.h"
#include "logger.h"
#include "scanner.h"
#include "stmt.h"

struct tokenlist {
    Token *curr;
};

static Stmt *statement(struct tokenlist *tlist);
static Stmt *print_stmt(struct tokenlist *tlist);
static Stmt *expr_stmt(struct tokenlist *tlist);
static Expr *expression(struct tokenlist *tlist);
static Expr *equality(struct tokenlist *tlist);
static Expr *comparison(struct tokenlist *tlist);
static Expr *addition(struct tokenlist *tlist);
static Expr *multiplication(struct tokenlist *tlist);
static Expr *unary(struct tokenlist *tlist);
static Expr *primary(struct tokenlist *tlist);


static Token *get_token(struct tokenlist *tlist)
{
    Token *prev;

    if ((prev = tlist->curr) == NULL)
        return NULL;

    prev = tlist->curr;
    tlist->curr = tlist->curr->next;

    return prev;
}


static Token *peek_token(struct tokenlist *tlist)
{
    return tlist->curr;
}


Stmt **parse(Token *tokens)
{
    int i;
    size_t stmt_size;
    struct tokenlist tlist;
    Stmt *stmt, **stmts;
    bool has_error;

    if (tokens == NULL)
        return NULL;

    tlist.curr = tokens;
    stmts = (Stmt **) malloc(128 * sizeof(Stmt *));

    has_error = false;
    for (i = 0; peek_token(&tlist) != NULL; i++)
        if ((stmt = statement(&tlist)) == NULL)
            has_error = true;
        else
            stmts[i++] = stmt;

    if (has_error) {
        while (--i) 
            free_stmt(stmts[i]);
        free(stmts);
        return NULL;
    }

    stmts[i] = NULL;

    return stmts;
}


static Stmt *statement(struct tokenlist *tlist)
{
    Token *token;
    Stmt *stmt;

    if ((token = peek_token(tlist)) == NULL)
        return NULL;

    if (token->type == TOKEN_PRINT) {
        get_token(tlist);
        stmt = print_stmt(tlist);
    } else {
        stmt = expr_stmt(tlist);
    }

    if ((token = get_token(tlist)) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected ';' at the end of statement");
        return NULL;
    }

    if (token->type != TOKEN_SEMICOLON) {
        log_error(LOX_SYNTAX_ERR, "expected ';' at the end of statement");
        return NULL;
    }

    return stmt;
}


static Stmt *print_stmt(struct tokenlist *tlist)
{
    Expr *expr;

    if ((expr = expression(tlist)) == NULL)
        return NULL;

    return new_print_stmt(expr);
}


static Stmt *expr_stmt(struct tokenlist *tlist)
{ 
    Expr *expr;

    if ((expr = expression(tlist)) == NULL)
        return NULL;

    return new_expr_stmt(expr);
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
        if (token->type == TOKEN_BANG_EQUAL || token->type == TOKEN_EQUAL_EQUAL) {
            get_token(tlist);

            if ((right = comparison(tlist)) == NULL) {
                return NULL;
            }

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
        if (token->type == TOKEN_MINUS || token->type == TOKEN_PLUS) {
            get_token(tlist);
            if ((right = multiplication(tlist)) == NULL) {
                return NULL;
            }
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

    if ((token = get_token(tlist)) == NULL)
        return NULL;

    if (token->type == TOKEN_LEFT_PAREN) {
        if ((expr = expression(tlist)) == NULL)
            return NULL;

        token = get_token(tlist);
        if (token == NULL || (token != NULL && token->type != TOKEN_RIGHT_PAREN)) {
            log_error(LOX_SYNTAX_ERR, "expected ')' after expression");
            return NULL;
        }

        return expr;
    }

    if (token->type == TOKEN_FALSE || token->type == TOKEN_NIL \
        || token->type == TOKEN_NUMBER || token->type == TOKEN_STRING \
        || token->type == TOKEN_TRUE)
        return new_literal_expr(token);

    log_error(LOX_SYNTAX_ERR, "invalid syntax");
    return NULL;
}
