#ifndef clox_value_h
#define clox_value_h

#include "common.h"


typedef double Value;


typedef struct {
    int capacity;
    int count;
    Value *values;
} ValueArray;


void Value_Print(Value value);

void ValueArray_Init(ValueArray *array);
void ValueArray_Write(ValueArray *array, Value value);
void ValueArray_Free(ValueArray *array);

#endif
