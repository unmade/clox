#include <stdlib.h>

#include "dict.h"
#include "environment.h"
#include "loxobj.h"


LoxEnv *new_env()
{
    return (LoxEnv *) new_dict();
}


void env_def(LoxEnv *env, char *name, LoxObj *obj)
{
    dict_set((Dict *) env, name, obj);
}


LoxObj *env_get(LoxEnv *env, char *name)
{
    return dict_get((Dict *) env, name);
}
