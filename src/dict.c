#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dict.h"

#define DEFAULT_SIZE 8

static unsigned hash_str(char *s);
static void dict_resize(Dict *dict);


Dict *new_dict()
{
    Dict *dict = (Dict *) malloc(sizeof(Dict));

    dict->capacity = DEFAULT_SIZE;
    dict->entries = (Entry **) calloc(dict->capacity, sizeof(Entry *));
    dict->fill = 0;
    dict->used = 0;

    return dict;
}



LoxObj *dict_get(Dict *dict, char *key)
{
    unsigned idx, hashval;
    Entry *entry;

    hashval = hash_str(key);
    idx = hashval % dict->capacity;

    while ((entry = dict->entries[idx]) != NULL)
        if ((entry->hashval == hashval) && (strcmp(entry->key, key) == 0))
            return entry->value;
        idx = (idx + 1) % dict->capacity;

    return NULL;
}


void dict_set(Dict *dict, char *key, LoxObj *value)
{
    unsigned idx, target_idx, hashval;
    bool seen;
    Entry *entry;

    seen = false;
    hashval = hash_str(key);
    idx = hashval % dict->capacity;

    while ((entry = dict->entries[idx]) != NULL) {
        if ((entry->hashval == hashval) && (strcmp(entry->key, key) == 0)) {
            seen = true;
            target_idx = idx;
            break;
        }
        if (!seen && entry->deleted)
            target_idx = idx;
        idx = (idx + 1) % dict->capacity;
    }

    if (!seen)
        target_idx = idx;

    if (dict->entries[target_idx] == NULL) {
        dict->fill++;
        dict->used++;
    } else if (dict->entries[target_idx]->deleted) {
        dict->used++;
    } else {
        free(dict->entries[target_idx]);
    }

    entry = (Entry *) malloc(sizeof(Entry));

    entry->key = key;
    entry->value = value;
    entry->hashval = hashval;
    entry->deleted = false;

    dict->entries[target_idx] = entry;

    if (3 * dict->fill >= 2 * dict->capacity)
        dict_resize(dict);
}


static void dict_resize(Dict *dict)
{
    unsigned i, idx;
    size_t capacity;
    Entry *entry, **entries;

    capacity = dict->capacity;
    if (capacity >= dict->used && dict->used > DEFAULT_SIZE)
        while (capacity >= dict->used)
            capacity /= 2;
    else
        while (capacity < dict->used)
            capacity *= 2;

    entries = (Entry **) calloc(capacity, sizeof(Entry));
    
    for (i = 0; i < dict->capacity; i++)
        if ((entry = dict->entries[i]) != NULL) {
            if (!entry->deleted) {
                idx = entry->hashval % capacity;
                while (entries[idx] != NULL)
                    idx = (idx + 1) % capacity;
                entries[idx] = entry;
            } else {
                dict->entries[i] = NULL;
                free(entry);
            }
        }

    free(dict->entries);

    dict->entries = entries;
    dict->capacity = capacity;
    dict->fill = dict->used;
    dict->used = capacity;
}


static unsigned hash_str(char *s) 
{
    unsigned hashval;

    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31 * hashval;

    return hashval;
}
