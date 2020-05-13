#ifndef clox_environment_h
#define clox_environment_h

#include "dict.h"
#include "loxobj.h"

typedef Dict LoxEnv;

LoxEnv *new_env();

void env_def(LoxEnv *env, char *name, LoxObj *obj);
LoxObj *env_get(LoxEnv *env, char *name);

#endif
