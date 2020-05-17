#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loxobj.h"


LoxObj *new_bool_obj(bool val)
{
    LoxObj *obj = (LoxObj *) malloc(sizeof(LoxObj));

    obj->type = LOX_OBJ_BOOL;
    obj->bval = val;

    return obj;
}


LoxObj *new_callable_obj(unsigned arity, func_t func)
{
    LoxObj *obj = (LoxObj *) malloc(sizeof(LoxObj));

    obj->type = LOX_OBJ_CALLABLE;
    obj->callable.arity = arity;
    obj->callable.func = func;

    return obj;
}


LoxObj *new_fun_obj(Stmt *declaration, unsigned arity)
{
    LoxObj *obj = (LoxObj *) malloc(sizeof(LoxObj));

    obj->type = LOX_OBJ_FUN;
    obj->fun.declaration = declaration;
    obj->fun.arity = arity;

    return obj;
}


LoxObj *new_nil_obj()
{
    LoxObj *obj = (LoxObj *) malloc(sizeof(LoxObj));

    obj->type = LOX_OBJ_NIL;
    
    return obj;
}


LoxObj *new_num_obj(float val)
{
    LoxObj *obj = (LoxObj *) malloc(sizeof(LoxObj));

    obj->type = LOX_OBJ_NUMBER;
    obj->fval = val;

    return obj;
}


LoxObj *new_str_obj(char *s)
{
    LoxObj *obj = (LoxObj *) malloc(sizeof(LoxObj));

    obj->type = LOX_OBJ_STRING;
    obj->sval = s;

    return obj;
}

    
void free_obj(LoxObj *obj)
{
    switch (obj->type) {
        case LOX_OBJ_STRING:
            free(obj->sval);
            break;
        default:
            break;
    }

    free(obj);
}


bool is_obj_truthy(const LoxObj *obj) 
{
    switch (obj->type) {
        case LOX_OBJ_BOOL:
            return (obj->bval == true);
        case LOX_OBJ_NIL:
            return false;
        default:
            return true;
    }
}


char *str_obj(const LoxObj *obj)
{
    char *s;
    size_t n;

    switch (obj->type) {
        case LOX_OBJ_BOOL:
            return strdup((obj->bval) ? "true" : "false");
        case LOX_OBJ_NUMBER:
            n = snprintf(NULL, 0, "%f", obj->fval);
            s = (char *) malloc(n * sizeof(char));
            sprintf(s, "%f", obj->fval);
            return s;
        case LOX_OBJ_STRING:
            return strdup(obj->sval);
        case LOX_OBJ_NIL:
            return strdup("nil");
        default:
            return NULL;
    }
}


void print_obj(const LoxObj *obj)
{
    char *s;

    s = str_obj(obj);
    printf("%s\n", s);
    free(s);
}
