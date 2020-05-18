#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "environment.h"
#include "expr.h"
#include "interpreter.h"
#include "logger.h"
#include "loxobj.h"
#include "scanner.h"
#include "stmt.h"

typedef struct {
    int code;
    LoxObj *value;
} ExecResult;


static ExecResult ExecResult_Ok()
{
    ExecResult res;

    res.code = 0;
    res.value = NULL;

    return res;
}


static ExecResult ExecResult_Return(LoxObj *obj)
{
    ExecResult res;

    res.code = 1;
    res.value = obj;

    return res;
}


static ExecResult ExecResult_Err()
{
    ExecResult res;

    res.code = -1;
    res.value = NULL;

    return res;
}


static LoxEnv *init_env();
static ExecResult exec(Stmt *stmt);
static ExecResult exec_block_stmt(Stmt *stmt);
static ExecResult exec_fun_stmt(Stmt *stmt);
static ExecResult exec_expr_stmt(Stmt *stmt);
static ExecResult exec_if_stmt(Stmt *stmt);
static ExecResult exec_print_stmt(Stmt *stmt);
static ExecResult exec_return_stmt(Stmt *stmt);
static ExecResult exec_var_stmt(Stmt *stmt);
static ExecResult exec_while_stmt(Stmt *stmt);
static LoxObj *eval(const Expr *expr);
static LoxObj *eval_assignment(const Expr *expr);
static LoxObj *eval_binary(const Expr *expr);
static LoxObj *eval_call(const Expr *expr);
static LoxObj *fun_call(LoxObj *self, unsigned argc, LoxObj **args);
static LoxObj *eval_literal(const Expr *expr);
static LoxObj *eval_unary(const Expr *expr);
static LoxObj *eval_var(const Expr *expr);
static char *joinstr(const char *s1, const char *s2);

static LoxEnv *ENV = NULL;


int interpret(Stmt **stmts)
{
    int i, code;

    if (ENV == NULL)
        ENV = init_env();

    for (i = 0; stmts[i] != NULL; i++)  {
        if ((code = exec(stmts[i]).code) < 0)
            return code;
    }

    return 0;
}


static LoxEnv *init_env()
{
    LoxEnv *env;
    char *s;

    env = new_env();
    
    s = strdup("clock");
    env_def(env, s, new_callable_obj(0, loxclock));

    return env;
}


static ExecResult exec(Stmt *stmt)
{
    switch (stmt->type) {
        case STMT_BLOCK:
            return exec_block_stmt(stmt);
        case STMT_EXPR:
            return exec_expr_stmt(stmt);
        case STMT_FUN:
            return exec_fun_stmt(stmt);
        case STMT_IF:
            return exec_if_stmt(stmt);
        case STMT_PRINT:
            return exec_print_stmt(stmt);
        case STMT_RETURN:
            return exec_return_stmt(stmt);
        case STMT_VAR:
            return exec_var_stmt(stmt);
        case STMT_WHILE:
            return exec_while_stmt(stmt);
        default:
            return ExecResult_Err();
    };
}


static ExecResult exec_block_stmt(Stmt *stmt)
{
    unsigned i;
    ExecResult res;

    ENV = enclose_env(ENV);

    for (i = 0; i < stmt->block.n; i++) {
        res = exec(stmt->block.stmts[i]);
        if (res.code != 0) {
            ENV = disclose_env(ENV);
            return res;
        }
    }

    ENV = disclose_env(ENV);

    return ExecResult_Ok();
}


static ExecResult exec_fun_stmt(Stmt *stmt)
{
    LoxObj *fun;

    fun = new_fun_obj(stmt, stmt->fun.n);

    env_def(ENV, stmt->fun.name, fun);

    return ExecResult_Ok();
}


static ExecResult exec_if_stmt(Stmt *stmt)
{
    LoxObj *cond;

    if ((cond = eval(stmt->ifelse.cond)) == NULL)
        return ExecResult_Err();

    if (is_obj_truthy(cond))
        return exec(stmt->ifelse.conseq);
    else if (stmt->ifelse.alt != NULL)
        return exec(stmt->ifelse.alt);
    else
        return ExecResult_Ok();
}


static ExecResult exec_print_stmt(Stmt *stmt)
{
    LoxObj *obj;

    if ((obj = eval(stmt->expr)) == NULL)
        return ExecResult_Err();

    print_obj(obj);

    return ExecResult_Ok();
}


static ExecResult exec_expr_stmt(Stmt *stmt)
{
    if (eval(stmt->expr) == NULL)
        return ExecResult_Err();
    else
        return ExecResult_Ok();
}


static ExecResult exec_return_stmt(Stmt *stmt)
{
    LoxObj *obj;

    if ((obj = eval(stmt->expr)) == NULL)
        return ExecResult_Err();
    else
        return ExecResult_Return(obj);
}


static ExecResult exec_var_stmt(Stmt *stmt)
{
    LoxObj *obj;

    if (stmt->var.expr != NULL) {
        if ((obj = eval(stmt->var.expr)) == NULL)
            return ExecResult_Err();
    } else {
        obj = new_nil_obj();
    }

    env_def(ENV, stmt->var.name, obj);

    return ExecResult_Ok();
}


static ExecResult exec_while_stmt(Stmt *stmt)
{
    ExecResult res;
    LoxObj *obj;

    while (true) {
        if (stmt->whileloop.cond != NULL) {
            if ((obj = eval(stmt->whileloop.cond)) == NULL)
                return ExecResult_Err();

            if (!is_obj_truthy(obj))
                return ExecResult_Ok();
        }

        if ((res = exec(stmt->whileloop.body)).code != 0)
            return res;
    }

    return ExecResult_Ok();
}


static LoxObj *eval(const Expr *expr)
{
    switch (expr->type) {
        case EXPR_ASSIGN:
            return eval_assignment(expr);
        case EXPR_BINARY:
            return eval_binary(expr);
        case EXPR_CALL:
            return eval_call(expr);
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
        case TOKEN_LESS:
            if (left->type == LOX_OBJ_NUMBER && right->type == LOX_OBJ_NUMBER)
                obj = new_bool_obj(left->fval < right->fval); 
            else
                log_error(LOX_RUNTIME_ERR, "operands must be numbers");
            break;
        case TOKEN_LESS_EQUAL:
            if (left->type == LOX_OBJ_NUMBER && right->type == LOX_OBJ_NUMBER)
                obj = new_bool_obj(left->fval <= right->fval); 
            else
                log_error(LOX_RUNTIME_ERR, "operands must be numbers");
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


static LoxObj *eval_call(const Expr *expr)
{
    unsigned i, arity;
    LoxObj *callee, *arg, **args, *obj;
    func_t f;

    args = NULL;

    if ((callee = eval(expr->call.callee)) == NULL)
        return NULL;

    switch (callee->type) {
        case LOX_OBJ_CALLABLE:
            f = callee->callable.func;
            arity = callee->callable.arity;
            break;
        case LOX_OBJ_FUN:
            f = fun_call;
            arity = callee->fun.arity;
            break;
        default:
            log_error(LOX_RUNTIME_ERR, "can only call functions or classes");
            goto cleanup;
    }

    if (expr->call.args != NULL ) {
        args = (LoxObj **) calloc(expr->call.argc + 1, sizeof(LoxObj *));
        for (i = 0; i < expr->call.argc; i++) {
            if ((arg = eval(expr->call.args[i])) == NULL)
                goto cleanup;
            args[i] = arg;
        }
    }

    if (expr->call.argc != arity) {
        log_error(LOX_RUNTIME_ERR, "too few or too many arguments");
        goto cleanup;
    }

    if ((obj = f(callee, expr->call.argc, args)) == NULL)
        goto cleanup;

    return obj;

cleanup:
    if (args != NULL) free(args);
    // maybe free obj later?
    return NULL;
}


static LoxObj *fun_call(LoxObj *self, unsigned argc, LoxObj **args)
{
    unsigned i;
    Stmt *block;
    ExecResult res;

    ENV = enclose_env(ENV);

    for (i = 0; i < argc; i++) {
        env_def(ENV, self->fun.declaration->fun.params[i]->lexeme, args[i]);
    }

    block = self->fun.declaration->fun.body;
    res = exec_block_stmt(block);

    ENV = disclose_env(ENV);

    if (res.code < 0)
        return NULL;

    if (res.value != NULL)
        return res.value;

    return new_nil_obj();
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
