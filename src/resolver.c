#include <stdbool.h>
#include <stdlib.h>

#include "dict.h"
#include "expr.h"
#include "logger.h"
#include "stmt.h"

bool FALSE = false;
bool TRUE = true;

typedef struct scope {
    struct scope *next;
    Dict *storage;
} Scope;


Scope *Scope_New()
{
    Scope *scope = (Scope *) malloc(sizeof(Scope));

    scope->next = NULL;
    scope->storage = Dict_New();

    return scope;
}


void Scope_Free(Scope *scope)
{
    Dict_Free(scope->storage);
    free(scope);
}


typedef struct {
    Scope *scopes;
    bool has_error;
} Resolver;


Resolver *Resolver_New()
{
    Resolver *resolver = (Resolver *) malloc(sizeof(Resolver)); 

    resolver->scopes = NULL;
    resolver->has_error = false;

    return resolver;
}


static void Resolver_Resolve_Stmt(Resolver *resolver, Stmt *stmt);
static void Resolver_Resolve_BlockStmt(Resolver *resolver, Stmt *stmt);
static void Resolver_Resolve_ExprStmt(Resolver *resolver, Stmt *stmt);
static void Resolver_Resolve_FunStmt(Resolver *resolver, Stmt *stmt);
static void Resolver_Resolve_IfStmt(Resolver *resolver, Stmt *stmt);
static void Resolver_Resolve_PrintStmt(Resolver *resolver, Stmt *stmt);
static void Resolver_Resolve_ReturnStmt(Resolver *resolver, Stmt *stmt);
static void Resolver_Resolve_VarStmt(Resolver *resolver, Stmt *stmt);
static void Resolver_Resolve_WhileStmt(Resolver *resolver, Stmt *stmt);

static void Resolver_Resolve_Expr(Resolver *resolver, Expr *expr);
static void Resolver_Resolve_AssignExpr(Resolver *resolver, Expr *expr);
static void Resolver_Resolve_BinaryExpr(Resolver *resolver, Expr *expr);
static void Resolver_Resolve_CallExpr(Resolver *resolver, Expr *expr);
static void Resolver_Resolve_GroupingExpr(Resolver *resolver, Expr *expr);
static void Resolver_Resolve_UnaryExpr(Resolver *resolver, Expr *expr);
static void Resolver_Resolve_VarExpr(Resolver *resolver, Expr *expr);

static void Resolver_ResolveLocal(Resolver *resolver, char *name, Expr *expr);

static void Resolver_BeginScope(Resolver *resolver);
static void Resolver_EndScope(Resolver *resolver);
static void Resolver_Declare(Resolver *resolver, char *name);
static void Resolver_Define(Resolver *resolver, char *name);


int resolve(Stmt **stmts)
{
    unsigned i;
    Resolver *resolver;

    resolver = Resolver_New();

    for (i = 0; stmts[i] != NULL; i++)
        Resolver_Resolve_Stmt(resolver, stmts[i]);

    return resolver->has_error;
}


static void Resolver_Resolve_Stmt(Resolver *resolver, Stmt *stmt)
{
    switch (stmt->type) {
        case STMT_BLOCK:
            return Resolver_Resolve_BlockStmt(resolver, stmt);
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


static void Resolver_Resolve_BlockStmt(Resolver *resolver, Stmt *stmt)
{
    unsigned i;

    Resolver_BeginScope(resolver);
    
    for (i = 0; i < stmt->block.n; i++)
        Resolver_Resolve_Stmt(resolver, stmt->block.stmts[i]);

    Resolver_EndScope(resolver);
}


static void Resolver_Resolve_ExprStmt(Resolver *resolver, Stmt *stmt)
{
    Resolver_Resolve_Expr(resolver, stmt->expr);
}


static void Resolver_Resolve_FunStmt(Resolver *resolver, Stmt *stmt)
{
    unsigned i;

    Resolver_Declare(resolver, stmt->fun.name);
    Resolver_Define(resolver, stmt->fun.name);

    Resolver_BeginScope(resolver);

    for (i = 0; i < stmt->fun.n; i++) {
        Resolver_Declare(resolver, stmt->fun.params[i]->lexeme);
        Resolver_Define(resolver, stmt->fun.params[i]->lexeme);
    }
    Resolver_Resolve_Stmt(resolver, stmt->fun.body);

    Resolver_EndScope(resolver);
}


static void Resolver_Resolve_IfStmt(Resolver *resolver, Stmt *stmt)
{
    Resolver_Resolve_Expr(resolver, stmt->ifelse.cond);
    Resolver_Resolve_Stmt(resolver, stmt->ifelse.conseq);
    if (stmt->ifelse.alt != NULL)
        Resolver_Resolve_Stmt(resolver, stmt->ifelse.alt);
}


static void Resolver_Resolve_PrintStmt(Resolver *resolver, Stmt *stmt)
{
    Resolver_Resolve_Expr(resolver, stmt->expr);
}


static void Resolver_Resolve_ReturnStmt(Resolver *resolver, Stmt *stmt)
{
    if (stmt->expr != NULL)
        Resolver_Resolve_Expr(resolver, stmt->expr);
}


static void Resolver_Resolve_VarStmt(Resolver *resolver, Stmt *stmt)
{
    Resolver_Declare(resolver, stmt->var.name);

    if (stmt->var.expr != NULL)
        Resolver_Resolve_Expr(resolver, stmt->var.expr);

    Resolver_Define(resolver, stmt->var.name);
}


static void Resolver_Resolve_WhileStmt(Resolver *resolver, Stmt *stmt)
{
    Resolver_Resolve_Expr(resolver, stmt->whileloop.cond);
    Resolver_Resolve_Stmt(resolver, stmt->whileloop.body);
}


static void Resolver_Resolve_Expr(Resolver *resolver, Expr *expr)
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


static void Resolver_Resolve_AssignExpr(Resolver *resolver, Expr *expr)
{
    Resolver_Resolve_Expr(resolver, expr->assign.value);
    Resolver_ResolveLocal(resolver, expr->assign.name->lexeme, expr);
}


static void Resolver_Resolve_BinaryExpr(Resolver *resolver, Expr *expr) 
{
    Resolver_Resolve_Expr(resolver, expr->binary.left);
    Resolver_Resolve_Expr(resolver, expr->binary.right);
}


static void Resolver_Resolve_CallExpr(Resolver *resolver, Expr *expr)
{
    unsigned i;

    Resolver_Resolve_Expr(resolver, expr->call.callee);

    for (i = 0; i < expr->call.argc; i++)
        Resolver_Resolve_Expr(resolver, expr->call.args[i]);
}


static void Resolver_Resolve_GroupingExpr(Resolver *resolver, Expr *expr)
{
    Resolver_Resolve_Expr(resolver, expr->grouping);
}


static void Resolver_Resolve_UnaryExpr(Resolver *resolver, Expr *expr)
{
    Resolver_Resolve_Expr(resolver, expr->unary.right);
}


static void Resolver_Resolve_VarExpr(Resolver *resolver, Expr *expr)
{
    bool *defined;

    if (resolver->scopes != NULL) {
        defined = DICT_GET(bool, resolver->scopes->storage, expr->varname->lexeme);
        if ((defined != NULL) && !*defined) {
            resolver->has_error = true;
            log_error(LOX_SYNTAX_ERR, "cannot read local variable in its own initializer");
            return;
        }
    }

    Resolver_ResolveLocal(resolver, expr->varname->lexeme, expr);

    return;
}


static void Resolver_ResolveLocal(Resolver *resolver, char *name, Expr *expr)
{
    unsigned i;
    Scope *scope;

    for (i = 0, scope = resolver->scopes; scope != NULL; scope = scope->next, i++) {
        if (DICT_GET(bool, scope->storage, name) != NULL) {
            expr->distance = i;
            // save position;
            return;
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


static void Resolver_Declare(Resolver *resolver, char *name)
{
    if (resolver->scopes != NULL)
        DICT_SET(resolver->scopes->storage, name, &FALSE);
}


static void Resolver_Define(Resolver *resolver, char *name)
{
    if (resolver->scopes != NULL)
        DICT_SET(resolver->scopes->storage, name, &TRUE);
}
