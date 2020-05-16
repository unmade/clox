#include <time.h>

#include "loxobj.h"

#define UNUSED(x) (void)(x)


LoxObj *loxclock(unsigned argc, LoxObj **args)
{
    UNUSED(argc);
    UNUSED(args);

    return new_num_obj((double) clock() / CLOCKS_PER_SEC);
}
