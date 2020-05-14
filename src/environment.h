#ifndef clox_environment_h
#define clox_environment_h

#include "dict.h"
#include "loxobj.h"

typedef struct loxenv {
    struct loxenv *next;
    Dict *storage;
} LoxEnv;

LoxEnv *new_env();
void free_env(LoxEnv *env);

LoxEnv *enclose_env(LoxEnv *env);
LoxEnv *disclose_env(LoxEnv *env);

int env_assign(LoxEnv *env, char *name, LoxObj *obj);
void env_def(LoxEnv *env, char *name, LoxObj *obj);
LoxObj *env_get(LoxEnv *env, char *name);

#endif
