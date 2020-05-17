#include <time.h>

#include "environment.h"
#include "loxobj.h"

#define UNUSED(x) (void)(x)


LoxObj *loxclock(LoxObj *self, unsigned argc, LoxObj **args)
{
    UNUSED(self);
    UNUSED(argc);
    UNUSED(args);

    return new_num_obj((double) clock() / CLOCKS_PER_SEC);
}
