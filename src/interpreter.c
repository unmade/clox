#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "environment.h"
#include "expr.h"
#include "interpreter.h"
#include "logger.h"
#include "loxobj.h"
#include "scanner.h"
#include "stmt.h"

static int exec(Stmt *stmt);
static int exec_block_stmt(Stmt *stmt);
static int exec_expr_stmt(Stmt *stmt);
static int exec_if_stmt(Stmt *stmt);
static int exec_print_stmt(Stmt *stmt);
static int exec_var_stmt(Stmt *stmt);
static int exec_while_stmt(Stmt *stmt);
static LoxObj *eval(const Expr *expr);
static LoxObj *eval_assignment(const Expr *expr);
static LoxObj *eval_binary(const Expr *expr);
static LoxObj *eval_literal(const Expr *expr);
static LoxObj *eval_unary(const Expr *expr);
static LoxObj *eval_var(const Expr *expr);
static char *joinstr(const char *s1, const char *s2);

static LoxEnv *ENV = NULL;


int interpret(Stmt **stmts)
{
    int i, code;

    if (ENV == NULL)
        ENV = new_env();

    for (i = 0; stmts[i] != NULL; i++)  {
        if ((code = exec(stmts[i])) != 0)
            return code;
    }

    return 0;
}


static int exec(Stmt *stmt)
{
    switch (stmt->type) {
        case STMT_BLOCK:
            return exec_block_stmt(stmt);
        case STMT_EXPR:
            return exec_expr_stmt(stmt);
        case STMT_IF:
            return exec_if_stmt(stmt);
        case STMT_PRINT:
            return exec_print_stmt(stmt);
        case STMT_VAR:
            return exec_var_stmt(stmt);
        case STMT_WHILE:
            return exec_while_stmt(stmt);
        default:
            return 1;
    };
}


static int exec_block_stmt(Stmt *stmt)
{
    unsigned i;

    ENV = enclose_env(ENV);

    for (i = 0; i < stmt->block.n; i++)
        if (exec(stmt->block.stmts[i]) != 0)
            return 1;

    ENV = disclose_env(ENV);

    return 0;
}



static int exec_if_stmt(Stmt *stmt)
{
    LoxObj *cond;

    if ((cond = eval(stmt->ifelse.cond)) == NULL)
        return 1;

    if (is_obj_truthy(cond))
        return exec(stmt->ifelse.conseq);
    else if (stmt->ifelse.alt != NULL)
        return exec(stmt->ifelse.alt);
    else
        return 0;
}


static int exec_print_stmt(Stmt *stmt)
{
    LoxObj *obj;

    if ((obj = eval(stmt->expr)) == NULL)
        return 1;

    print_obj(obj);

    return 0;
}


static int exec_expr_stmt(Stmt *stmt)
{
    return (eval(stmt->expr) == NULL);
}


static int exec_var_stmt(Stmt *stmt)
{
    LoxObj *obj;

    if (stmt->var.expr != NULL) {
        if ((obj = eval(stmt->var.expr)) == NULL)
            return 1;
    } else {
        obj = new_nil_obj();
    }

    env_def(ENV, stmt->var.name, obj);

    return 0;
}


static int exec_while_stmt(Stmt *stmt)
{
    int res;
    LoxObj *obj;

    while (true) {
        if ((obj = eval(stmt->whileloop.cond)) == NULL)
            return 1;

        if (!is_obj_truthy(obj))
            return 0;

        if ((res = exec(stmt->whileloop.body)) != 0)
            return res;
    }

    return 0;
}


static LoxObj *eval(const Expr *expr)
{
    switch (expr->type) {
        case EXPR_ASSIGN:
            return eval_assignment(expr);
        case EXPR_BINARY:
            return eval_binary(expr);
        case EXPR_GROUPING:
            return eval(expr->grouping);
        case EXPR_LITERAL:
            return eval_literal(expr);
        case EXPR_UNARY:
            return eval_unary(expr);
        case EXPR_VAR:
            return eval_var(expr);
        default:
            return NULL;
    }
}


static LoxObj *eval_assignment(const Expr *expr)
{
    LoxObj *value;

    if ((value = eval(expr->assign.value)) != NULL) {
        env_assign(ENV, expr->varname->lexeme, value);
        return value;
    }

    return NULL;
}


static LoxObj *eval_unary(const Expr *expr) 
{
    LoxObj *right;

    if ((right = eval(expr->unary.right)) == NULL)
        return NULL;

    switch (expr->unary.op->type) {
        case TOKEN_BANG:
            return new_bool_obj(!is_obj_truthy(right));
        case TOKEN_MINUS:
            if (right->type != LOX_OBJ_NUMBER) {
                log_error(LOX_RUNTIME_ERR, "operand must be a number");
                return NULL; 
            }
            return new_num_obj(right->fval * -1);
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
            else
                log_error(LOX_RUNTIME_ERR, "operands must be numbers");
            break;
        case TOKEN_SLASH:
            if (left->type == LOX_OBJ_NUMBER && right->type == LOX_OBJ_NUMBER)
                obj = new_num_obj(left->fval / right->fval);
            else
                log_error(LOX_RUNTIME_ERR, "operands must be numbers");
            break;
        case TOKEN_STAR:
            if (left->type == LOX_OBJ_NUMBER && right->type == LOX_OBJ_NUMBER)
                obj = new_num_obj(left->fval * right->fval); 
            else
                log_error(LOX_RUNTIME_ERR, "operands must be numbers");
            break;
        case TOKEN_PLUS:
            if (left->type == LOX_OBJ_NUMBER && right->type == LOX_OBJ_NUMBER)
                obj = new_num_obj(left->fval + right->fval);
            else if (left->type == LOX_OBJ_STRING && right->type == LOX_OBJ_STRING)
                obj = new_str_obj(joinstr(left->sval, right->sval));
            else
                log_error(LOX_RUNTIME_ERR, "operands must be two numbers or two strings");
            break;
        default:
            log_error(LOX_RUNTIME_ERR, "unexpected binary operator");
            break;
    }

    // I can't free objs, cause they might be referenced by others
    //free_obj(left);
    //free_obj(right);

    return obj;
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


static LoxObj *eval_var(const Expr *expr)
{
    LoxObj *obj;

    if ((obj = env_get(ENV, expr->varname->lexeme)) == NULL)
        log_error(LOX_RUNTIME_ERR, "undefined variable '%s'", expr->varname->lexeme);

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
