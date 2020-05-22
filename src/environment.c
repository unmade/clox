#include <stdlib.h>

#include "dict.h"
#include "environment.h"
#include "logger.h"
#include "loxobj.h"


LoxEnv *new_env()
{
    LoxEnv *env = (LoxEnv *) malloc(sizeof(LoxEnv));

    env->next = NULL;
    env->storage = Dict_New();

    return env;
}


void free_env(LoxEnv *env)
{
    Dict_Free(env->storage);
    free(env);
}


LoxEnv *enclose_env(LoxEnv *env)
{
    LoxEnv *local_env = new_env();

    local_env->next = env;

    return local_env;
}


LoxEnv *disclose_env(LoxEnv *env)
{
    LoxEnv *e = env->next;

    env->next = NULL;
    free_env(env);

    return e;
}


int env_assign(LoxEnv *env, char *name, LoxObj *obj)
{
    LoxEnv *e;
    LoxObj *o;

    for (e = env; e != NULL; e = e->next)
        if ((o = DICT_GET(LoxObj, e->storage, name)) != NULL) {
            DICT_SET(e->storage, name, obj);
            return 0;
        }

    log_error(LOX_RUNTIME_ERR, "undefined variable '%s'", name);
    return 1;
}


void env_def(LoxEnv *env, char *name, LoxObj *obj)
{
    DICT_SET(env->storage, name, obj);
}


LoxObj *env_get(LoxEnv *env, char *name)
{
    LoxEnv *e;
    LoxObj *o;

    for (e = env; e != NULL; e = e->next)
        if ((o = DICT_GET(LoxObj, e->storage, name)) != NULL)
            return o;

    return NULL;
}
