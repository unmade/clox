#include <stdlib.h>

#include "dict.h"
#include "environment.h"
#include "logger.h"
#include "loxobj.h"


LoxEnv *new_env()
{
    return (LoxEnv *) new_dict();
}


int env_assign(LoxEnv *env, char *name, LoxObj *obj)
{
    if (env_get(env, name) == NULL) {
        log_error(LOX_RUNTIME_ERR, "undefined variable '%s'", name);
        return 1;
    }

    dict_set((Dict *) env, name, obj);
    return 0;
}


void env_def(LoxEnv *env, char *name, LoxObj *obj)
{
    dict_set((Dict *) env, name, obj);
}


LoxObj *env_get(LoxEnv *env, char *name)
{
    return dict_get((Dict *) env, name);
}
