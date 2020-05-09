#ifndef clox_loxobj_h
#define clox_loxobj_h

#include <stdbool.h>

enum LoxObjType {
    LOX_OBJ_BOOL = 0,
    LOX_OBJ_NIL,
    LOX_OBJ_NUMBER,
    LOX_OBJ_STRING,
};


typedef struct {
    enum LoxObjType type;
    union {
        bool bval;
        float fval;
        char *sval;
    };
} LoxObj;


LoxObj *new_bool_obj(bool val);
LoxObj *new_nil_obj();
LoxObj *new_num_obj(float val);
LoxObj *new_str_obj(char *s);

void free_obj(LoxObj *obj);

bool is_obj_truthy(const LoxObj *obj);

char *str_obj(const LoxObj *obj);
void print_obj(const LoxObj *obj);

#endif
