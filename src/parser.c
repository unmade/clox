#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"
#include "logger.h"
#include "parser.h"
#include "scanner.h"
#include "stmt.h"

#define MAX_CALL_ARGS 254

struct tokenlist {
    Token *curr;
};

static Stmt *declaration(struct tokenlist *tlist);
static Stmt *class_declaration(struct tokenlist *tlist);
static Stmt *fun_declaration(struct tokenlist *tlist);
static Stmt *var_declaration(struct tokenlist *tlist);
static Stmt *statement(struct tokenlist *tlist);
static Stmt *for_stmt(struct tokenlist *tlist);
static Stmt *print_stmt(struct tokenlist *tlist);
static Stmt *return_stmt(struct tokenlist *tlist);
static Stmt *block_stmt(struct tokenlist *tlist);
static Stmt *if_stmt(struct tokenlist *tlist);
static Stmt *while_stmt(struct tokenlist *tlist);
static Stmt *expr_stmt(struct tokenlist *tlist);
static Expr *expression(struct tokenlist *tlist);
static Expr *assignment(struct tokenlist *tlist);
static Expr *equality(struct tokenlist *tlist);
static Expr *comparison(struct tokenlist *tlist);
static Expr *addition(struct tokenlist *tlist);
static Expr *multiplication(struct tokenlist *tlist);
static Expr *unary(struct tokenlist *tlist);
static Expr *call(struct tokenlist *tlist);
static Expr *finish_call(struct tokenlist *tlist, Expr *expr);
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


static Token *take_token(struct tokenlist *tlist, TokenType tok_type)
{
    if (peek_token(tlist) == NULL)
        return NULL;
    if (peek_token(tlist)->type != tok_type)
        return NULL;
    return get_token(tlist);
}


Stmt **parse(Token *tokens)
{
    unsigned i;
    size_t n;
    bool has_error;
    struct tokenlist tlist;
    Stmt *stmt, **stmts;

    tlist.curr = tokens;

    n = 1;
    stmts = (Stmt **) calloc(n, sizeof(Stmt *));

    has_error = false;
    for (i = 0; peek_token(&tlist) != NULL; i++) {
        if ((stmt = declaration(&tlist)) == NULL) {
            has_error = true;
            break;
        }
        if (i >= n - 1)
            stmts = (Stmt **) realloc(stmts, sizeof(Stmt *) * (n *= 2));
        stmts[i] = stmt;
    }

    if (has_error) {
        free_stmts(stmts);
        return NULL;
    }

    stmts[i] = NULL;

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
    if (take_token(tlist, TOKEN_CLASS) != NULL)
        return class_declaration(tlist);

    if (take_token(tlist, TOKEN_FUN) != NULL)
        return fun_declaration(tlist);

    if (take_token(tlist, TOKEN_VAR) != NULL)
        return var_declaration(tlist);

    return statement(tlist);
}


static Stmt *class_declaration(struct tokenlist *tlist)
{
    size_t i, n;
    Token *name, *token;
    Stmt *method, **methods;

    if ((name = take_token(tlist, TOKEN_IDENTIFIER)) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected class name");
        return NULL;
    }

    if (take_token(tlist, TOKEN_LEFT_BRACE) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected '{' before class body");
        return NULL;
    }

    i = 0;
    n = 1; 
    methods = (Stmt **) calloc(n, sizeof(Stmt *));

    while ((token = peek_token(tlist)) != NULL && token->type != TOKEN_RIGHT_BRACE) {
        if ((method = fun_declaration(tlist)) == NULL)
            goto cleanup;
        if (i >= n)
            methods = (Stmt **) realloc(methods, sizeof(Stmt *) * (n *= 2));
        methods[i++] = method;
    }

    if (take_token(tlist, TOKEN_RIGHT_BRACE) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected '}' after class body");
        goto cleanup;
    }

    return new_klass_stmt(name, i, methods);

cleanup:
    n = i;
    for (i = 0; i < n; i++)
        free_stmt(methods[i]);
    free(methods);

    return NULL;
}


static Stmt *fun_declaration(struct tokenlist *tlist)
{
    char *name;
    size_t n;
    unsigned i;
    Token *token, **params, *start;
    Stmt *body;

    i = n = 0;
    params = NULL;

    if ((token = take_token(tlist, TOKEN_IDENTIFIER)) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected function name after 'fun'");
        return NULL;
    }

    name = token->lexeme;

    if (take_token(tlist, TOKEN_LEFT_PAREN) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected '(' after function name");
        return NULL;
    }

    token = peek_token(tlist);
    if (token != NULL && token->type != TOKEN_RIGHT_PAREN) {

        // estimate how much space to allocate for parameters
        start = get_token(tlist);
        do {
            token = get_token(tlist);
            if (token != NULL && token->type == TOKEN_IDENTIFIER)
                n++;
        } while (take_token(tlist, TOKEN_COMMA) != NULL);

        // fill up parameters
        tlist->curr = start;
        params = (Token **) calloc(n, sizeof(Token *));
        do {
            token = get_token(tlist);
            if (token != NULL && token->type != TOKEN_IDENTIFIER) {
                log_error(LOX_SYNTAX_ERR, "expected parameter name");
                goto cleanup;
            }
            params[i++] = token;
        } while (take_token(tlist, TOKEN_COMMA) != NULL);
    }

    if (i > MAX_CALL_ARGS) {
        log_error(LOX_SYNTAX_ERR, "cannot have more than %d parameters", MAX_CALL_ARGS);
        goto cleanup;
    }

    if (take_token(tlist, TOKEN_RIGHT_PAREN) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected ')' after parameters");
        goto cleanup;
    }

    if (take_token(tlist, TOKEN_LEFT_BRACE) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected '{' before function body");
        goto cleanup;
    }

    if ((body = block_stmt(tlist)) == NULL)
        goto cleanup;

    return new_fun_stmt(strdup(name), i, params, body);

cleanup:
    if (params != NULL) free(params);

    return NULL;
}



static Stmt *var_declaration(struct tokenlist *tlist)
{
    char *name;
    Token *token;
    Expr *expr;

    if ((token = take_token(tlist, TOKEN_IDENTIFIER)) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected variable name after 'var'");
        return NULL;
    }

    name = token->lexeme;
    expr = NULL;

    if (take_token(tlist, TOKEN_EQUAL) != NULL)
        if ((expr = expression(tlist)) == NULL)
            return NULL;

    if (take_token(tlist, TOKEN_SEMICOLON) == NULL) {
        free_expr(expr);
        log_error(LOX_SYNTAX_ERR, "expected ';' at the end of statement");
        return NULL;
    }

    return new_var_stmt(strdup(name), expr);
}


static Stmt *statement(struct tokenlist *tlist)
{
    Token *token;

    if ((token = peek_token(tlist)) == NULL)
        return NULL;

    switch (token->type) {
        case TOKEN_FOR:
            get_token(tlist);
            return for_stmt(tlist);
        case TOKEN_LEFT_BRACE:
            get_token(tlist);
            return block_stmt(tlist);
        case TOKEN_IF:
            get_token(tlist);
            return if_stmt(tlist);
        case TOKEN_PRINT:
            get_token(tlist);
            return print_stmt(tlist);
        case TOKEN_RETURN:
            get_token(tlist);
            return return_stmt(tlist);
        case TOKEN_WHILE:
            get_token(tlist);
            return while_stmt(tlist);
        default:
            return expr_stmt(tlist);
    }
}


static Stmt *for_stmt(struct tokenlist *tlist)
{
    Expr *cond, *inc;
    Stmt *init, *body, **stmts;

    init = body = NULL;
    cond = inc = NULL;

    if (take_token(tlist, TOKEN_LEFT_PAREN) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected '(' after 'for'");
        return NULL;
    }

    /* loop initialization */

    if (take_token(tlist, TOKEN_SEMICOLON) != NULL) {
        init = NULL;
    } else if (take_token(tlist, TOKEN_VAR) != NULL) {
        if ((init = var_declaration(tlist)) == NULL)
            return NULL;
    } else {
        if ((init = expr_stmt(tlist)) == NULL)
            return NULL;
    }

    /* loop condition */

    if (take_token(tlist, TOKEN_SEMICOLON) == NULL)
        if ((cond = expression(tlist)) == NULL)
            goto cleanup;
    
    if (take_token(tlist, TOKEN_SEMICOLON) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected ';' after loop condition");
        goto cleanup;
    }

    /* loop increment */

    if (take_token(tlist, TOKEN_RIGHT_PAREN) == NULL)
        if ((inc = expression(tlist)) == NULL)
            goto cleanup;

    if (take_token(tlist, TOKEN_RIGHT_PAREN) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected ')' after for clauses");
        goto cleanup;
    }

    /* loop body */

    if ((body = statement(tlist)) == NULL)
        goto cleanup;

    /* transform 'for' loop to 'while' */

    if (inc != NULL) {
        stmts = (Stmt **) calloc(2, sizeof(Stmt *));
        stmts[0] = body;
        stmts[1] = new_expr_stmt(inc);
        body = new_block_stmt(2, stmts);
    }

    body = new_while_stmt(cond, body);

    if (init != NULL) {
        stmts = (Stmt **) calloc(2, sizeof(Stmt *));
        stmts[0] = init;
        stmts[1] = body;
        body = new_block_stmt(2, stmts);
    }

    return body;

cleanup:
    if (init != NULL) free_stmt(init);
    if (cond != NULL) free_expr(cond);
    if (inc != NULL) free_expr(inc);
    if (body != NULL) free_stmt(body);

    return NULL;
}


static Stmt *print_stmt(struct tokenlist *tlist)
{
    Expr *expr;

    if ((expr = expression(tlist)) == NULL)
        return NULL;

    if (take_token(tlist, TOKEN_SEMICOLON) == NULL) {
        free_expr(expr);
        log_error(LOX_SYNTAX_ERR, "expected ';' at the end of statement");
        return NULL;
    } 

    return new_print_stmt(expr);
}


static Stmt *return_stmt(struct tokenlist *tlist)
{
    Expr *expr;

    if ((expr = expression(tlist)) == NULL)
        return NULL;

    if (take_token(tlist, TOKEN_SEMICOLON) == NULL) {
        free_expr(expr);
        log_error(LOX_SYNTAX_ERR, "expected ';' at the end of statement");
        return NULL;
    } 

    return new_return_stmt(expr);
}


static Stmt *block_stmt(struct tokenlist *tlist)
{
    size_t n;
    unsigned i;
    bool has_error;

    Token *t;
    Stmt *stmt, **stmts;

    n = 1;
    i = 0;
    has_error = false;
    stmts = (Stmt **) calloc(n, sizeof(Stmt *));

    while (!has_error && (t = peek_token(tlist)) != NULL && t->type != TOKEN_RIGHT_BRACE) {
        if ((stmt = declaration(tlist)) == NULL)
            has_error = true;
        if (i >= n)
            stmts = (Stmt **) realloc(stmts, sizeof(Stmt *) * (n *= 2));
        stmts[i++] = stmt;
    }

    if (has_error)
        goto cleanup; 

    if (take_token(tlist, TOKEN_RIGHT_BRACE) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected '}' at the end of the block");
        goto cleanup;
    }

    return new_block_stmt(i, stmts);

cleanup:
    n = i;
    for (i = 0; i < n; i++)
        free_stmt(stmts[i]);
    free(stmts);

    return NULL;
}


static Stmt *if_stmt(struct tokenlist *tlist)
{
    Expr *cond;
    Stmt *conseq, *alt;

    cond = NULL;
    conseq = alt = NULL;

    if (take_token(tlist, TOKEN_LEFT_PAREN) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected '(' after 'if'");
        return NULL;
    }

    if ((cond = expression(tlist)) == NULL)
        return NULL;

    if (take_token(tlist, TOKEN_RIGHT_PAREN) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected ')' after if condition");
        goto cleanup;
    }

    if ((conseq = statement(tlist)) == NULL)
        goto cleanup;

    if (take_token(tlist, TOKEN_ELSE))
        if ((alt = statement(tlist)) == NULL)
            goto cleanup;

    return new_if_stmt(cond, conseq, alt); 

cleanup:
    if (cond != NULL) free_expr(cond);
    if (conseq != NULL) free_stmt(conseq);
    if (alt != NULL) free_stmt(alt);

    return NULL;
}


static Stmt *while_stmt(struct tokenlist *tlist)
{
    Expr *cond;
    Stmt *body;

    cond = NULL;
    body = NULL;

    if (take_token(tlist, TOKEN_LEFT_PAREN) == NULL) { 
        log_error(LOX_SYNTAX_ERR, "expected '(' after 'while'");
        return NULL;
    }

    if ((cond = expression(tlist)) == NULL)
        return NULL;

    if (take_token(tlist, TOKEN_RIGHT_PAREN) == NULL) {
        log_error(LOX_SYNTAX_ERR, "expected ')' after while condition");
        goto cleanup;
    }

    if ((body = statement(tlist)) == NULL)
        goto cleanup;

    return new_while_stmt(cond, body);

cleanup:
    if (cond != NULL) free_expr(cond);
    if (body != NULL) free_stmt(body);

    return NULL;
}


static Stmt *expr_stmt(struct tokenlist *tlist)
{ 
    Expr *expr;

    if ((expr = expression(tlist)) == NULL)
        return NULL;

    if ((take_token(tlist, TOKEN_SEMICOLON)) == NULL) {
        free_expr(expr);
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
    Expr *expr, *rexpr;
    
    expr = rexpr = NULL;

    if ((expr = equality(tlist)) == NULL)
        return NULL;

    if ((take_token(tlist, TOKEN_EQUAL)) != NULL) {
        if ((rexpr = assignment(tlist)) == NULL)
            goto cleanup;

        if (expr->type == EXPR_VAR)
            return new_assign_expr(expr->varname, rexpr);

        if (expr->type == EXPR_GET)
            return new_set_expr(expr->get.name, expr->get.object, rexpr);

        log_error(LOX_SYNTAX_ERR, "invalid assignment target");
        goto cleanup;
    }

    return expr;

cleanup:
    if (expr != NULL) free_expr(expr);
    if (rexpr != NULL) free_expr(rexpr);

    return NULL;
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
                free_expr(expr);
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

            if ((right = addition(tlist)) == NULL) {
                free_expr(expr);
                return NULL;
            }
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
                free_expr(expr);
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
            if ((right = unary(tlist)) == NULL) {
                free_expr(expr);
                return NULL;
            }
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

    return call(tlist);
}


static Expr *call(struct tokenlist *tlist)
{
    Token *name;
    Expr *expr, *temp;

    if ((expr = primary(tlist)) == NULL)
        return NULL;
    
    for (;;) {
        if (take_token(tlist, TOKEN_LEFT_PAREN) != NULL) {
            if ((temp = finish_call(tlist, expr)) == NULL) {
                free_expr(expr);
                return NULL;
            }
            expr = temp;
        } else if (take_token(tlist, TOKEN_DOT)) {
            if ((name = take_token(tlist, TOKEN_IDENTIFIER)) == NULL) {
                free_expr(expr);
                log_error(LOX_SYNTAX_ERR, "expect property name after '.'");
                return NULL;
            }
            expr = new_get_expr(name, expr);
        } else {
            break;
        }
    }

    return expr;
}


static Expr *finish_call(struct tokenlist *tlist, Expr *expr)
{
    size_t n;
    unsigned i;
    Token *token, *first;
    Expr *arg, **args;

    args = NULL;
    i = n = 0;

    first = peek_token(tlist);
    if ((token = peek_token(tlist)) != NULL && token->type != TOKEN_RIGHT_PAREN) {
        n = 1;
        while ((token = get_token(tlist)) != NULL && token->type != TOKEN_RIGHT_PAREN) {
            if (token->type == TOKEN_COMMA)
                n++;
        }
    }
    tlist->curr = first;

    if (n > 0) {
        args = (Expr **) calloc(n, sizeof(Expr *));

        do {
            if ((arg = expression(tlist)) == NULL)
                goto cleanup;
            args[i++] = arg;
        } while (take_token(tlist, TOKEN_COMMA) != NULL);
    }
 
    if (i > MAX_CALL_ARGS) {
        log_error(LOX_SYNTAX_ERR, "cannot have more than %d arguments", MAX_CALL_ARGS);
        goto cleanup;
    }

    if (token == NULL || (token != NULL && token->type != TOKEN_RIGHT_PAREN)) {
        log_error(LOX_SYNTAX_ERR, "expected ')' after arguments");
        goto cleanup;
    }
 
    get_token(tlist);

    return new_call_expr(expr, token, i, args);

cleanup:
    n = i;
    for (i = 0; i < n; i++)
        free_expr(args[i]);
    if (args != NULL) free(args);

    return NULL;
}


static Expr *primary(struct tokenlist *tlist)
{
    Expr *expr;
    Token *token;

    expr = NULL;

    if ((token = get_token(tlist)) == NULL) {
        log_error(LOX_SYNTAX_ERR, "unexpected EOF");
        return NULL;
    }

    if (token->type == TOKEN_LEFT_PAREN) {
        if ((expr = expression(tlist)) == NULL)
            return NULL;

        if (take_token(tlist, TOKEN_RIGHT_PAREN) == NULL) {
            free_expr(expr);
            log_error(LOX_SYNTAX_ERR, "expected ')' after expression");
            return NULL;
        }

        return expr;
    }

    if (token->type == TOKEN_FALSE || token->type == TOKEN_NIL \
        || token->type == TOKEN_NUMBER || token->type == TOKEN_STRING \
        || token->type == TOKEN_TRUE)
        return new_literal_expr(token);

    if (token->type == TOKEN_THIS)
        return new_this_expr(token);

    if (token->type == TOKEN_IDENTIFIER)
        return new_var_expr(token);

    free_expr(expr);
    log_error(LOX_SYNTAX_ERR, "invalid syntax");
    return NULL;
}
