#ifndef clox_dict_h
#define clox_dict_h

#include <stdbool.h>
#include <stdlib.h>

#define DICT_GET(type, dict, name) (type *) Dict_Get(dict, name)
#define DICT_SET(dict, name, value) Dict_Set(dict, name, (void *) value)

typedef struct {
    char *key;
    void *value;
    unsigned hashval;
    bool deleted;
} Entry;


typedef struct {
    Entry **entries;
    size_t capacity;
    unsigned fill;
    unsigned used;
} Dict;

Dict *Dict_New();
void Dict_Free();

void *Dict_Get(Dict *d, char *key);
void Dict_Set(Dict *d, char *key, void *value);

#endif
