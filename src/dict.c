#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dict.h"

#define DEFAULT_SIZE 8

static Entry **new_entries(size_t n);
static void free_entries(size_t n, Entry **entries);

static Entry *Entry_New(char *key, void *obj, unsigned hashval);
static Entry *Entry_Copy(Entry *entry);
static void Entry_Free(Entry *entry);

static unsigned hash_str(char *s);
static void Dict_Resize(Dict *dict);


Dict *Dict_New()
{
    Dict *dict = (Dict *) malloc(sizeof(Dict));

    dict->capacity = DEFAULT_SIZE;
    dict->entries = new_entries(dict->capacity);
    dict->fill = 0;
    dict->used = 0;

    return dict;
}


Dict *Dict_Copy(Dict *dict)
{
    unsigned i;
    Dict *copy;

    copy = (Dict *) malloc(sizeof(Dict));

    copy->capacity = dict->capacity;
    copy->fill = dict->fill;
    copy->used = dict->used;

    copy->entries = new_entries(copy->capacity);
    for (i = 0; i < copy->capacity; i++)
        if (dict->entries[i] != NULL)
            copy->entries[i] = Entry_Copy(dict->entries[i]);

    return copy;
}


void Dict_Free(Dict *dict)
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
            Entry_Free(entry);
        entries[i] = NULL;
    }
}


static Entry *Entry_New(char *key, void *value, unsigned hashval)
{ 
    Entry *entry;

    entry = (Entry *) malloc(sizeof(Entry));

    entry->key = strdup(key);
    entry->value = value;
    entry->hashval = hashval;
    entry->deleted = false;

    return entry;
}


static Entry *Entry_Copy(Entry *entry)
{
    Entry *copy;

    copy = (Entry *) malloc(sizeof(Entry));

    copy->key = strdup(entry->key);
    copy->value = entry->value;
    copy->hashval = entry->hashval;
    copy->deleted = entry->deleted;

    return copy;
}


static void Entry_Free(Entry *entry)
{
    free(entry->key);
    // I don't think dict should free its values
    // free(entry->value);
    free(entry);
}


void *Dict_Get(Dict *dict, char *key)
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


void Dict_Set(Dict *dict, char *key, void *value)
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
        dict->entries[target_idx] = Entry_New(key, value, hashval);
    } else if (dict->entries[target_idx]->deleted) {
        dict->used++;
        dict->entries[target_idx]->value = value;
        dict->entries[target_idx]->deleted = false;
    } else {
        dict->entries[target_idx]->value = value;
    }
 
    if (3 * dict->fill >= 2 * dict->capacity)
        Dict_Resize(dict);
}


static void Dict_Resize(Dict *dict)
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
                Entry_Free(entry);
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
