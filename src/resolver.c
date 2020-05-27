#include <stdbool.h>
#include <stdlib.h>

#include "dict.h"
#include "expr.h"
#include "logger.h"
#include "stmt.h"

bool FALSE = false;
bool TRUE = true;

enum FunType {
    FUN_NONE,
    FUN_FUN,
};


typedef struct scope {
    struct scope *next;
    Dict *storage;
} Scope;


typedef struct {
    Scope *scopes;
    bool has_error;
    enum FunType curr_fun;
} Resolver;


static Resolver *Resolver_New();
static void Resolver_Free(Resolver *resolver);

static Scope *Scope_New();
static void Scope_Free();

static void Resolver_Resolve_Stmt(Resolver *resolver, const Stmt *stmt);
static void Resolver_Resolve_BlockStmt(Resolver *resolver, const Stmt *stmt);
static void Resolver_Resolve_ClassStmt(Resolver *resolver, const Stmt *stmt);
static void Resolver_Resolve_ExprStmt(Resolver *resolver, const Stmt *stmt);
static void Resolver_Resolve_FunStmt(Resolver *resolver, const Stmt *stmt);
static void Resolver_Resolve_IfStmt(Resolver *resolver, const Stmt *stmt);
static void Resolver_Resolve_PrintStmt(Resolver *resolver, const Stmt *stmt);
static void Resolver_Resolve_ReturnStmt(Resolver *resolver, const Stmt *stmt);
static void Resolver_Resolve_VarStmt(Resolver *resolver, const Stmt *stmt);
static void Resolver_Resolve_WhileStmt(Resolver *resolver, const Stmt *stmt);

static void Resolver_Resolve_Expr(Resolver *resolver, const Expr *expr);
static void Resolver_Resolve_AssignExpr(Resolver *resolver, const Expr *expr);
static void Resolver_Resolve_BinaryExpr(Resolver *resolver, const Expr *expr);
static void Resolver_Resolve_CallExpr(Resolver *resolver, const Expr *expr);
static void Resolver_Resolve_GroupingExpr(Resolver *resolver, const Expr *expr);
static void Resolver_Resolve_UnaryExpr(Resolver *resolver, const Expr *expr);
static void Resolver_Resolve_VarExpr(Resolver *resolver, const Expr *expr);

static void Resolver_BeginScope(Resolver *resolver);
static void Resolver_EndScope(Resolver *resolver);
static void Resolver_Declare(Resolver *resolver, const char *name);
static void Resolver_Define(Resolver *resolver, const char *name);


int resolve(Stmt **stmts)
{
    bool has_error;
    unsigned i;
    Resolver *resolver;

    resolver = Resolver_New();

    for (i = 0; stmts[i] != NULL; i++)
        Resolver_Resolve_Stmt(resolver, stmts[i]);

    has_error = resolver->has_error;

    Resolver_Free(resolver);

    return has_error;
}



static Scope *Scope_New()
{
    Scope *scope = (Scope *) malloc(sizeof(Scope));

    scope->next = NULL;
    scope->storage = Dict_New();

    return scope;
}


static void Scope_Free(Scope *scope)
{
    Dict_Free(scope->storage);
    free(scope);
}


static Resolver *Resolver_New()
{
    Resolver *resolver = (Resolver *) malloc(sizeof(Resolver)); 

    resolver->scopes = NULL;
    resolver->has_error = false;
    resolver->curr_fun = FUN_NONE;

    return resolver;
}


static void Resolver_Free(Resolver *resolver)
{
    Scope *scope;

    for (scope = resolver->scopes; scope != NULL; scope = resolver->scopes->next)
        Scope_Free(scope);
    
    free(resolver);
}


static void Resolver_Resolve_Stmt(Resolver *resolver, const Stmt *stmt)
{
    switch (stmt->type) {
        case STMT_BLOCK:
            return Resolver_Resolve_BlockStmt(resolver, stmt);
        case STMT_CLASS:
            return Resolver_Resolve_ClassStmt(resolver, stmt);
        case STMT_EXPR:
            return Resolver_Resolve_ExprStmt(resolver, stmt);
        case STMT_FUN:
            return Resolver_Resolve_FunStmt(resolver, stmt);
        case STMT_IF:
            return Resolver_Resolve_IfStmt(resolver, stmt);
        case STMT_PRINT:
            return Resolver_Resolve_PrintStmt(resolver, stmt);
        case STMT_RETURN:
            return Resolver_Resolve_ReturnStmt(resolver, stmt);
        case STMT_VAR:
            return Resolver_Resolve_VarStmt(resolver, stmt);
        case STMT_WHILE:
            return Resolver_Resolve_WhileStmt(resolver, stmt);
    }
}


static void Resolver_Resolve_BlockStmt(Resolver *resolver, const Stmt *stmt)
{
    unsigned i;

    Resolver_BeginScope(resolver);
    
    for (i = 0; i < stmt->block.n; i++)
        Resolver_Resolve_Stmt(resolver, stmt->block.stmts[i]);

    Resolver_EndScope(resolver);
}


static void Resolver_Resolve_ClassStmt(Resolver *resolver, const Stmt *stmt)
{
    Resolver_Declare(resolver, stmt->klass.name->lexeme);
    Resolver_Define(resolver, stmt->klass.name->lexeme);
}


static void Resolver_Resolve_ExprStmt(Resolver *resolver, const Stmt *stmt)
{
    Resolver_Resolve_Expr(resolver, stmt->expr);
}


static void Resolver_Resolve_FunStmt(Resolver *resolver, const Stmt *stmt)
{
    unsigned i;
    enum FunType curr_fun;

    Resolver_Declare(resolver, stmt->fun.name);
    Resolver_Define(resolver, stmt->fun.name);

    curr_fun = resolver->curr_fun;
    resolver->curr_fun = FUN_FUN;

    Resolver_BeginScope(resolver);

    for (i = 0; i < stmt->fun.n; i++) {
        Resolver_Declare(resolver, stmt->fun.params[i]->lexeme);
        Resolver_Define(resolver, stmt->fun.params[i]->lexeme);
    }
    Resolver_Resolve_Stmt(resolver, stmt->fun.body);

    resolver->curr_fun = curr_fun;

    Resolver_EndScope(resolver);
}


static void Resolver_Resolve_IfStmt(Resolver *resolver, const Stmt *stmt)
{
    Resolver_Resolve_Expr(resolver, stmt->ifelse.cond);
    Resolver_Resolve_Stmt(resolver, stmt->ifelse.conseq);
    if (stmt->ifelse.alt != NULL)
        Resolver_Resolve_Stmt(resolver, stmt->ifelse.alt);
}


static void Resolver_Resolve_PrintStmt(Resolver *resolver, const Stmt *stmt)
{
    Resolver_Resolve_Expr(resolver, stmt->expr);
}


static void Resolver_Resolve_ReturnStmt(Resolver *resolver, const Stmt *stmt)
{
    if (resolver->curr_fun == FUN_NONE) {
        resolver->has_error = true;
        log_error(LOX_SYNTAX_ERR, "cannot return from top-level code");
        return;
    }
    if (stmt->expr != NULL)
        Resolver_Resolve_Expr(resolver, stmt->expr);
}


static void Resolver_Resolve_VarStmt(Resolver *resolver, const Stmt *stmt)
{
    Resolver_Declare(resolver, stmt->var.name);

    if (stmt->var.expr != NULL)
        Resolver_Resolve_Expr(resolver, stmt->var.expr);

    Resolver_Define(resolver, stmt->var.name);
}


static void Resolver_Resolve_WhileStmt(Resolver *resolver, const Stmt *stmt)
{
    Resolver_Resolve_Expr(resolver, stmt->whileloop.cond);
    Resolver_Resolve_Stmt(resolver, stmt->whileloop.body);
}


static void Resolver_Resolve_Expr(Resolver *resolver, const Expr *expr)
{
    switch (expr->type) {
        case EXPR_ASSIGN:
            return Resolver_Resolve_AssignExpr(resolver, expr);
        case EXPR_BINARY:
            return Resolver_Resolve_BinaryExpr(resolver, expr);
        case EXPR_CALL:
            return Resolver_Resolve_CallExpr(resolver, expr);
        case EXPR_GROUPING:
            return Resolver_Resolve_GroupingExpr(resolver, expr);
        case EXPR_UNARY:
            return Resolver_Resolve_UnaryExpr(resolver, expr);
        case EXPR_VAR:
            return Resolver_Resolve_VarExpr(resolver, expr);
        default:
            break;
    }
}


static void Resolver_Resolve_AssignExpr(Resolver *resolver, const Expr *expr)
{
    Resolver_Resolve_Expr(resolver, expr->assign.value);
}


static void Resolver_Resolve_BinaryExpr(Resolver *resolver, const Expr *expr) 
{
    Resolver_Resolve_Expr(resolver, expr->binary.left);
    Resolver_Resolve_Expr(resolver, expr->binary.right);
}


static void Resolver_Resolve_CallExpr(Resolver *resolver, const Expr *expr)
{
    unsigned i;

    Resolver_Resolve_Expr(resolver, expr->call.callee);

    for (i = 0; i < expr->call.argc; i++)
        Resolver_Resolve_Expr(resolver, expr->call.args[i]);
}


static void Resolver_Resolve_GroupingExpr(Resolver *resolver, const Expr *expr)
{
    Resolver_Resolve_Expr(resolver, expr->grouping);
}


static void Resolver_Resolve_UnaryExpr(Resolver *resolver, const Expr *expr)
{
    Resolver_Resolve_Expr(resolver, expr->unary.right);
}


static void Resolver_Resolve_VarExpr(Resolver *resolver, const Expr *expr)
{
    bool *defined;

    if (resolver->scopes != NULL) {
        defined = DICT_GET(bool, resolver->scopes->storage, expr->varname->lexeme);
        if ((defined != NULL) && !*defined) {
            resolver->has_error = true;
            log_error(LOX_SYNTAX_ERR, "cannot read local variable in its own initializer");
        }
    }
}


static void Resolver_BeginScope(Resolver *resolver)
{
    Scope *scope;

    scope = Scope_New();

    scope->next = resolver->scopes;
    resolver->scopes = scope;
}


static void Resolver_EndScope(Resolver *resolver)
{
    Scope *scope;

    scope = resolver->scopes;
    resolver->scopes = scope->next;

    Scope_Free(scope);
}


static void Resolver_Declare(Resolver *resolver, const char *name)
{
    if (resolver->scopes != NULL) {
        if (DICT_GET(bool, resolver->scopes->storage, (char *) name) == NULL) {
            DICT_SET(resolver->scopes->storage, (char *) name, &FALSE);
        } else {
            log_error(LOX_SYNTAX_ERR, 
                      "Variable with name '%s' already declared in this scope", name);
            resolver->has_error = true;
        }
    }
}


static void Resolver_Define(Resolver *resolver, const char *name)
{
    if (resolver->scopes != NULL)
        DICT_SET(resolver->scopes->storage, (char *) name, &TRUE);
}
