#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dict.h"

#define DEFAULT_SIZE 8

static Entry **new_entries(size_t n);
static void free_entries(size_t n, Entry **entries);

static Entry *new_entry(char *key, LoxObj *obj, unsigned hashval);
static void free_entry(Entry *entry);

static unsigned hash_str(char *s);
static void dict_resize(Dict *dict);


Dict *new_dict()
{
    Dict *dict = (Dict *) malloc(sizeof(Dict));

    dict->capacity = DEFAULT_SIZE;
    dict->entries = new_entries(dict->capacity);
    dict->fill = 0;
    dict->used = 0;

    return dict;
}


void free_dict(Dict *dict)
{
    free_entries(dict->capacity, dict->entries);
    free(dict);
}

static Entry **new_entries(size_t n)
{
    return (Entry **) calloc(n, sizeof(Entry *));
}


static void free_entries(size_t n, Entry **entries)
{
    unsigned i;
    Entry *entry;

    for (i = 0; i < n; i++) {
        if ((entry = entries[i]) != NULL)
            free_entry(entry);
        entries[i] = NULL;
    }
}


static Entry *new_entry(char *key, LoxObj *value, unsigned hashval)
{ 
    Entry *entry;

    entry = (Entry *) malloc(sizeof(Entry));

    entry->key = strdup(key);
    entry->value = value;
    entry->hashval = hashval;
    entry->deleted = false;

    return entry;
}


static void free_entry(Entry *entry)
{
    free(entry->key);
    // I don't think dict should free its values
    // free(entry->value);
    free(entry);
}


LoxObj *dict_get(Dict *dict, char *key)
{
    unsigned idx, hashval;
    Entry *entry;

    hashval = hash_str(key);
    idx = hashval % dict->capacity;

    while ((entry = dict->entries[idx]) != NULL) {
        if (!entry->deleted && entry->hashval == hashval && strcmp(entry->key, key) == 0)
            return entry->value;
        idx = (idx + 1) % dict->capacity;
    }

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
        dict->entries[target_idx] = new_entry(key, value, hashval);
    } else if (dict->entries[target_idx]->deleted) {
        dict->used++;
        dict->entries[target_idx]->value = value;
        dict->entries[target_idx]->deleted = false;
    } else {
        dict->entries[target_idx]->value = value;
    }
 
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

    entries = new_entries(capacity);
    
    for (i = 0; i < dict->capacity; i++)
        if ((entry = dict->entries[i]) != NULL) {
            if (!entry->deleted) {
                idx = entry->hashval % capacity;
                while (entries[idx] != NULL)
                    idx = (idx + 1) % capacity;
                entries[idx] = entry;
            } else {
                dict->entries[i] = NULL;
                free_entry(entry);
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
