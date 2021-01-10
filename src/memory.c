#include <stdlib.h>

#include "memory.h"


void *reallocate(void *pointer, size_t old_size, size_t new_size)
{
    void *result;

    if (new_size == 0) {
        free(pointer);
        return NULL;
    }

    result = realloc(pointer, new_size);
    if (result == NULL) {
        exit(1);
    }

    return result;
}
