#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"
#include "logger.h"
#include "parser.h"
#include "scanner.h"
#include "stmt.h"

struct tokenlist {
    Token *curr;
};

static Stmt *declaration(struct tokenlist *tlist);
static Stmt *var_declaration(struct tokenlist *tlist);
static Stmt *statement(struct tokenlist *tlist);
static Stmt *print_stmt(struct tokenlist *tlist);
static Stmt *block_stmt(struct tokenlist *tlist);
static Stmt *if_stmt(struct tokenlist *tlist);
static Stmt *expr_stmt(struct tokenlist *tlist);
static Expr *expression(struct tokenlist *tlist);
static Expr *assignment(struct tokenlist *tlist);
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
    size_t n;
    bool has_error;
    struct tokenlist tlist;
    Stmt *stmt, **stmts;
    Token *token;

    if (tokens == NULL)
        return NULL;

    n = 1;
    for (token = tokens; token != NULL; token = token->next)
        if (token->type == TOKEN_SEMICOLON)
            n++;
    stmts = (Stmt **) calloc(n + 1, sizeof(Stmt *));

    tlist.curr = tokens;

    has_error = false;
    for (i = 0; peek_token(&tlist) != NULL; i++)
        if ((stmt = declaration(&tlist)) == NULL)
            has_error = true;
        else
            stmts[i] = stmt;

    if (has_error) {
        free_stmts(stmts);
        return NULL;
    }

    stmts[i++] = NULL;

    return stmts;
}


void free_stmts(Stmt **stmts)
{
    unsigned i;

    for (i = 0; stmts[i] != NULL; i++)
        free_stmt(stmts[i]);

    free(stmts);
}


static Stmt *declaration(struct tokenlist *tlist)
{
    int type;

    Token *token;
    Stmt *stmt;

    if ((token = peek_token(tlist)) == NULL)
        return NULL;

    if ((type = token->type) == TOKEN_VAR) {
        get_token(tlist);
        stmt = var_declaration(tlist);
    } else {
        stmt = statement(tlist);
    }

    return stmt;
}


static Stmt *var_declaration(struct tokenlist *tlist)
{
    char *name;
    Token *token;
    Expr *expr;

    token = get_token(tlist);
    if (token == NULL || (token != NULL && token->type != TOKEN_IDENTIFIER)) {
        log_error(LOX_SYNTAX_ERR, "expected variable name after 'var'");
        return NULL;
    }

    name = strdup(token->lexeme);

    if ((token = peek_token(tlist)) == NULL) 
        return NULL;

    expr = NULL;
    if (token->type == TOKEN_EQUAL) {
        get_token(tlist);
        if ((expr = expression(tlist)) == NULL)
            return NULL;
    }

    if ((token = get_token(tlist)) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected ';' at the end of statement");
        return NULL;
    }

    if (token->type != TOKEN_SEMICOLON) {
        log_error(LOX_SYNTAX_ERR, "expected ';' at the end of statement");
        return NULL;
    } 

    return new_var_stmt(name, expr);
}


static Stmt *statement(struct tokenlist *tlist)
{
    Token *token;

    if ((token = peek_token(tlist)) == NULL)
        return NULL;

    switch (token->type) {
        case TOKEN_LEFT_BRACE:
            get_token(tlist);
            return block_stmt(tlist);
        case TOKEN_IF:
            get_token(tlist);
            return if_stmt(tlist);
        case TOKEN_PRINT:
            get_token(tlist);
            return print_stmt(tlist);
        default:
            return expr_stmt(tlist);
    }
}


static Stmt *print_stmt(struct tokenlist *tlist)
{
    Token *token;
    Expr *expr;

    if ((expr = expression(tlist)) == NULL)
        return NULL;

    if ((token = get_token(tlist)) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected ';' at the end of statement");
        return NULL;
    }

    if (token->type != TOKEN_SEMICOLON) {
        log_error(LOX_SYNTAX_ERR, "expected ';' at the end of statement");
        return NULL;
    } 

    return new_print_stmt(expr);
}


static Stmt *block_stmt(struct tokenlist *tlist)
{
    size_t n;
    unsigned i;
    bool has_error;

    Stmt *stmt, **stmts;
    Token *first, *t;

    n = 0;
    first = get_token(tlist);
    for (t = first; t != NULL && t->type != TOKEN_RIGHT_BRACE; t = get_token(tlist)) {
        if (t->type == TOKEN_SEMICOLON)
            n++;
    }

    if (t == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected '}' at the end of the block");
        return NULL;
    }

    stmts = (Stmt **) calloc(n + 1, sizeof(Stmt *));

    i = 0;
    has_error = false;
    tlist->curr = first;
    while ((t = peek_token(tlist)) != NULL && t->type != TOKEN_RIGHT_BRACE) {
        if ((stmt = declaration(tlist)) == NULL)
            has_error = true;
        else
            stmts[i++] = stmt;
    }

    if (has_error) {
        n = i;
        for (i = 0; i < n; i++)
            free_stmt(stmts[i]);
        free(stmts);
        return NULL;
    }

    return new_block_stmt(i, stmts);
}

static Stmt *if_stmt(struct tokenlist *tlist)
{
    Token *token;
    Expr *cond;
    Stmt *conseq, *alt;

    if ((token = get_token(tlist)) == NULL)
        return NULL;

    if ((token->type != TOKEN_LEFT_PAREN)) {
        log_error(LOX_SYNTAX_ERR, "expected '(' after 'if'");
        return NULL;
    }

    if ((cond = expression(tlist)) == NULL)
        return NULL;

    token = get_token(tlist);
    if ((token == NULL || (token != NULL && token->type != TOKEN_RIGHT_PAREN))) {
        log_error(LOX_SYNTAX_ERR, "expected ')' after if condition");
        return NULL;
    }

    if ((conseq = statement(tlist)) == NULL)
        return NULL;

    alt = NULL; 
    if ((token = peek_token(tlist)) != NULL && token->type == TOKEN_ELSE) {
        get_token(tlist);
        alt = statement(tlist);
    }

    return new_if_stmt(cond, conseq, alt); 
}


static Stmt *expr_stmt(struct tokenlist *tlist)
{ 
    Token *token;
    Expr *expr;

    if ((expr = expression(tlist)) == NULL)
        return NULL;

    if ((token = get_token(tlist)) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected ';' at the end of statement");
        return NULL;
    }

    if (token->type != TOKEN_SEMICOLON) {
        log_error(LOX_SYNTAX_ERR, "expected ';' at the end of statement");
        return NULL;
    } 

    return new_expr_stmt(expr);
}


static Expr *expression(struct tokenlist *tlist)
{
    return assignment(tlist);
}


static Expr *assignment(struct tokenlist *tlist)
{
    Token *token;
    Expr *expr, *rexpr;
    
    if ((expr = equality(tlist)) == NULL)
        return NULL;

    if ((token = peek_token(tlist)) != NULL)
        if (token->type == TOKEN_EQUAL) {
            get_token(tlist);
            
            if ((rexpr = assignment(tlist)) == NULL)
                return NULL;

            if (expr->type != EXPR_VAR) {
                log_error(LOX_SYNTAX_ERR, "invalid assignment target");
                return NULL;
            }

            return new_assign_expr(expr->varname, rexpr);
        }

    return expr;
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

    if ((token = get_token(tlist)) == NULL) {
        log_error(LOX_SYNTAX_ERR, "unexpected EOF");
        return NULL;
    }

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

    if (token->type == TOKEN_IDENTIFIER)
        return new_var_expr(token);

    log_error(LOX_SYNTAX_ERR, "invalid syntax");
    return NULL;
}
