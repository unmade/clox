#include <stdlib.h>

#include "dict.h"
#include "environment.h"
#include "logger.h"
#include "loxobj.h"


LoxEnv *new_env()
{
    LoxEnv *env = (LoxEnv *) malloc(sizeof(LoxEnv));

    env->next = NULL;
    env->storage = new_dict();

    return env;
}


void free_env(LoxEnv *env)
{
    free_dict(env->storage);
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
        if ((o = dict_get(e->storage, name)) != NULL) {
            dict_set(e->storage, name, obj);
            return 0;
        }

    log_error(LOX_RUNTIME_ERR, "undefined variable '%s'", name);
    return 1;
}


void env_def(LoxEnv *env, char *name, LoxObj *obj)
{
    dict_set(env->storage, name, obj);
}


LoxObj *env_get(LoxEnv *env, char *name)
{
    LoxEnv *e;
    LoxObj *o;

    for (e = env; e != NULL; e = e->next)
        if ((o = dict_get(e->storage, name)) != NULL)
            return o;

    return NULL;
}
