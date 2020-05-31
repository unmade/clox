#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dict.h"
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


LoxObj *new_class_obj(char *name, LoxObj *superclass, Dict *methods)
{
    LoxObj *obj = (LoxObj *) malloc(sizeof(LoxObj));

    obj->type = LOX_OBJ_CLASS;
    obj->klass.name = strdup(name);
    obj->klass.superclass = superclass;
    obj->klass.methods = methods;
    
    return obj;
}


LoxObj *new_fun_obj(Stmt *declaration, unsigned arity, bool init)
{
    LoxObj *obj = (LoxObj *) malloc(sizeof(LoxObj));

    obj->type = LOX_OBJ_FUN;
    obj->fun.declaration = declaration;
    obj->fun.arity = arity;
    obj->fun.init = init;
    obj->fun.closure = NULL;

    return obj;
}


LoxObj *new_instance_obj(LoxObj *klass)
{
    LoxObj *obj = (LoxObj *) malloc(sizeof(LoxObj));

    obj->type = LOX_OBJ_INSTANCE;
    obj->instance.klass = klass;
    obj->instance.fields = Dict_New();

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


bool is_obj_equal(const LoxObj *a, const LoxObj *b)
{
    if (a->type == LOX_OBJ_NIL && b->type == LOX_OBJ_NIL)
        return true;
    if (a->type == LOX_OBJ_NIL)
        return false;
    if (a->type != b->type)
        return false;
    
    if (a->type == LOX_OBJ_BOOL)
        return a->bval == b->bval;
    if (a->type == LOX_OBJ_NUMBER)
        return a->fval == b->fval;
    if (a->type == LOX_OBJ_STRING)
        return strcmp(a->sval, b->sval) == 0;

    return false;
}


char *str_obj(const LoxObj *obj)
{
    char *s;
    size_t n;

    switch (obj->type) {
        case LOX_OBJ_BOOL:
            return strdup((obj->bval) ? "true" : "false");
        case LOX_OBJ_CLASS:
            return strdup(obj->klass.name);
        case LOX_OBJ_INSTANCE:
            n = strlen(obj->instance.klass->klass.name) + strlen("instance");
            s = (char *) calloc(n + 1, sizeof(char));
            sprintf(s, "%s %s", obj->instance.klass->klass.name, "instance");
            return s;
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
