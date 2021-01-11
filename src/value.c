#include <stdio.h>

#include "memory.h"
#include "value.h"


void Value_Print(Value value)
{
    printf("%g", value);
}


void ValueArray_Init(ValueArray *array) 
{
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}


void ValueArray_Write(ValueArray *array, Value value) 
{
    int old_capacity;

    if (array->capacity < array->count + 1) {
        old_capacity = array->capacity;
        array->capacity = GROW_CAPACITY(old_capacity);
        array->values = GROW_ARRAY(Value, array->values, old_capacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}


void ValueArray_Free(ValueArray *array) 
{
    FREE_ARRAY(Value, array->values, array->capacity);
    ValueArray_Init(array);
}
