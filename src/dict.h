#ifndef clox_dict_h
#define clox_dict_h

#include <stdbool.h>
#include <stdlib.h>

#include "loxobj.h"

typedef struct {
    char *key;
    LoxObj *value;
    unsigned hashval;
    bool deleted;
} Entry;


typedef struct {
    Entry **entries;
    size_t capacity;
    unsigned fill;
    unsigned used;
} Dict;


Dict *new_dict();
void free_dict(Dict *d);

LoxObj *dict_get(Dict *d, char *key);
void dict_set(Dict *d, char *key, LoxObj *value);

#endif
