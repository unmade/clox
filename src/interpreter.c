#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dict.h"
#include "environment.h"
#include "expr.h"
#include "globals.h"
#include "interpreter.h"
#include "logger.h"
#include "loxobj.h"
#include "scanner.h"
#include "stmt.h"

#define UNUSED(x) (void)(x)

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
static ExecResult exec_class_stmt(Stmt *stmt);
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
static unsigned class_arity(LoxObj *self);
static LoxObj *class_call(LoxObj *self, unsigned argc, LoxObj **args);
static LoxObj *fun_call(LoxObj *self, unsigned argc, LoxObj **args);
static LoxObj *eval_get(const Expr *expr);
static LoxObj *eval_literal(const Expr *expr);
static LoxObj *eval_logic(const Expr *expr);
static LoxObj *eval_this(const Expr *expr);
static LoxObj *eval_set(const Expr *expr);
static LoxObj *eval_super(const Expr *expr);
static LoxObj *eval_unary(const Expr *expr);
static LoxObj *eval_var(const Expr *expr);
static char *joinstr(const char *s1, const char *s2);
static LoxObj *find_method(LoxObj *obj, LoxObj *klass, char *name);

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
        case STMT_CLASS:
            return exec_class_stmt(stmt);
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


static ExecResult exec_class_stmt(Stmt *stmt)
{
    bool init;
    unsigned i;
    LoxObj *klass, *method, *superclass;
    Dict *methods;

    superclass = NULL;
    if (stmt->klass.superclass != NULL) {
        if ((superclass = eval(stmt->klass.superclass)) == NULL)
            return ExecResult_Err();

        if (superclass->type != LOX_OBJ_CLASS) {
            log_error(LOX_RUNTIME_ERR, "superclass must be a class");
            return ExecResult_Err();
        }
    }

    env_def(ENV, stmt->klass.name->lexeme, new_nil_obj());

    if (superclass != NULL) {
        ENV = enclose_env(ENV);
        env_def(ENV, "super", superclass);
    }

    methods = Dict_New(); 
    for (i = 0; i < stmt->klass.n; i++) {
        init = (strcmp(stmt->klass.methods[i]->fun.name, "init") == 0);
        method = new_fun_obj(stmt->klass.methods[i], stmt->klass.methods[i]->fun.n, init);
        DICT_SET(methods, method->fun.declaration->fun.name, method);
    }

    klass = new_class_obj(stmt->klass.name->lexeme, superclass, methods);

    if (superclass != NULL)
        env_assign(ENV->next, stmt->klass.name->lexeme, klass);
    else
        env_assign(ENV, stmt->klass.name->lexeme, klass);

    if (ENV->next != NULL) {
        for (i = 0; i< stmt->klass.n; i++) {
            method = DICT_GET(LoxObj, klass->klass.methods, stmt->klass.methods[i]->fun.name);
            method->fun.closure = env_copy(ENV); 
        }
    }

    if (superclass != NULL)
        ENV = disclose_env(ENV);

    return ExecResult_Ok();
}


static ExecResult exec_fun_stmt(Stmt *stmt)
{
    LoxObj *fun;

    fun = new_fun_obj(stmt, stmt->fun.n, false);
    if (ENV->next != NULL)
        fun->fun.closure = env_copy(ENV);

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

    if (stmt->expr == NULL)
        return ExecResult_Return(new_nil_obj());

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
        case EXPR_GET:
            return eval_get(expr);
        case EXPR_GROUPING:
            return eval(expr->grouping);
        case EXPR_LITERAL:
            return eval_literal(expr);
        case EXPR_LOGIC:
            return eval_logic(expr);
        case EXPR_THIS:
            return eval_this(expr);
        case EXPR_SET:
            return eval_set(expr);
        case EXPR_SUPER:
            return eval_super(expr);
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


static LoxObj *eval_logic(const Expr *expr)
{
    LoxObj *left;

    if ((left = eval(expr->binary.left)) == NULL)
        return NULL;

    if (expr->binary.op->type == TOKEN_OR) {
        if (is_obj_truthy(left))
            return left;
    } else {
        if (!is_obj_truthy(left))
            return left;
    }

    return eval(expr->binary.right);
}


static LoxObj *eval_this(const Expr *expr)
{
    LoxObj *obj;

    if ((obj = env_get(ENV, expr->keyword->lexeme)) == NULL)
        log_error(LOX_RUNTIME_ERR, "undefined variable '%s'", expr->keyword->lexeme);

    return obj;
}


static LoxObj *eval_set(const Expr *expr)
{
    LoxObj *obj, *value;

    if ((obj = eval(expr->set.object)) == NULL)
        return NULL;

    if (obj->type != LOX_OBJ_INSTANCE) {
        log_error(LOX_RUNTIME_ERR, "only instances have fields");
        return NULL;
    }

    if ((value = eval(expr->set.value)) == NULL)
        return NULL;

    DICT_SET(obj->instance.fields, expr->set.name->lexeme, value);

    return value;
}


static LoxObj *eval_super(const Expr *expr)
{
    LoxObj *instance, *superclass, *method;

    instance = env_get(ENV, "this");
    superclass = env_get(ENV, "super");

    method = find_method(instance, superclass, expr->super.method->lexeme);

    if (method == NULL) {
        log_error(LOX_RUNTIME_ERR, "undefined property '%s'", expr->super.method->lexeme);
        return NULL;
    }

    return method;
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
        case TOKEN_BANG_EQUAL:
            obj = new_bool_obj(!is_obj_equal(left, right));
            break;
        case TOKEN_EQUAL_EQUAL:
            obj = new_bool_obj(is_obj_equal(left, right));
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
        case TOKEN_GREATER:
            if (left->type == LOX_OBJ_NUMBER && right->type == LOX_OBJ_NUMBER)
                obj = new_bool_obj(left->fval > right->fval);
            else
                log_error(LOX_RUNTIME_ERR, "operands must be numbers");
            break;
        case TOKEN_GREATER_EQUAL:
            if (left->type == LOX_OBJ_NUMBER && right->type == LOX_OBJ_NUMBER)
                obj = new_bool_obj(left->fval >= right->fval);
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
        case LOX_OBJ_CLASS:
            f = class_call;
            arity = class_arity(callee);
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
        log_error(LOX_RUNTIME_ERR, "expected %u arguments, got %u", arity, expr->call.argc);
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


static LoxObj *class_call(LoxObj *self, unsigned argc, LoxObj **args)
{
    LoxObj *instance, *init;

    instance = new_instance_obj(self);

    if ((init = find_method(instance, instance->instance.klass, "init")) != NULL)
        return fun_call(init, argc, args);

    return instance;
}


static unsigned class_arity(LoxObj *self)
{
    LoxObj *init;

    if ((init = DICT_GET(LoxObj, self->klass.methods, "init")) != NULL)
        return init->fun.arity;
    if (self->klass.superclass != NULL)
        return class_arity(self->klass.superclass);
    return 0;
}


static LoxObj *fun_call(LoxObj *self, unsigned argc, LoxObj **args)
{
    unsigned i;
    Stmt *block;
    ExecResult res;

    LoxEnv *env = ENV;
    if (self->fun.closure != NULL)
        ENV = enclose_env(self->fun.closure);
    else
        ENV = enclose_env(ENV);

    for (i = 0; i < argc; i++) {
        env_def(ENV, self->fun.declaration->fun.params[i]->lexeme, args[i]);
    }

    block = self->fun.declaration->fun.body;
    res = exec_block_stmt(block);

    ENV = disclose_env(ENV);
    ENV = env;

    if (res.code < 0)
        return NULL;

    if (self->fun.init)
        return env_get(self->fun.closure, "this");

    if (res.value != NULL)
        return res.value;

    return new_nil_obj();
}


static LoxObj *eval_get(const Expr *expr)
{
    char *name;
    LoxObj *obj, *prop, *method;

    if ((obj = eval(expr->get.object)) == NULL)
        return NULL;

    if ((obj->type != LOX_OBJ_INSTANCE)) {
        log_error(LOX_RUNTIME_ERR, "only instances have properties");
        return NULL;
    }

    name = expr->get.name->lexeme;

    if ((prop = DICT_GET(LoxObj, obj->instance.fields, name)) != NULL)
        return prop;

    if ((method = find_method(obj, obj->instance.klass, name)) != NULL)
        return method;

    log_error(LOX_RUNTIME_ERR, "undefined property '%s'", name);
    return NULL;
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


static LoxObj *find_method(LoxObj *obj, LoxObj *klass, char *name)
{
    LoxEnv *env;
    LoxObj *prop, *method;

    method = NULL;
    if ((prop = DICT_GET(LoxObj, klass->klass.methods, name)) != NULL) {
        method = new_fun_obj(prop->fun.declaration, prop->fun.arity, prop->fun.init);

        if (prop->fun.closure != NULL)
            env = enclose_env(prop->fun.closure);
        else
            env = enclose_env(ENV);
        
        env_def(env, "this", obj);
        method->fun.closure = env;
    }

    if (method == NULL && klass->klass.superclass != NULL)
        return find_method(obj, klass->klass.superclass, name);

    return method;
}
