#ifndef clox_loxobj_h
#define clox_loxobj_h

#include <stdbool.h>

#include "stmt.h"

struct loxenv;  // forward declaration for LoxEnv

enum LoxObjType {
    LOX_OBJ_BOOL = 0,
    LOX_OBJ_CALLABLE,
    LOX_OBJ_FUN,
    LOX_OBJ_NIL,
    LOX_OBJ_NUMBER,
    LOX_OBJ_STRING,
};

struct loxobj;

typedef struct loxobj *(*func_t)(struct loxobj *self, unsigned argc, struct loxobj **args);

typedef struct loxobj {
    enum LoxObjType type;
    union {
        bool bval;
        float fval;
        char *sval;
        struct {
            struct loxenv *closure;
            Stmt *declaration;
            unsigned arity;
        } fun;
        struct {
            unsigned arity;
            func_t func;
        } callable;
    };
} LoxObj;


LoxObj *new_bool_obj(bool val);
LoxObj *new_callable_obj(unsigned arity, func_t func);
LoxObj *new_fun_obj(Stmt *declaration, unsigned arity);
LoxObj *new_nil_obj();
LoxObj *new_num_obj(float val);
LoxObj *new_str_obj(char *s);

void free_obj(LoxObj *obj);

bool is_obj_truthy(const LoxObj *obj);

char *str_obj(const LoxObj *obj);
void print_obj(const LoxObj *obj);

#endif
