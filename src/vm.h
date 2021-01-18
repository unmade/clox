#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"


typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;


typedef struct {
    Chunk *chunk;
    uint8_t *ip;
    Value *stack;
    Value *stack_top;
    int stack_capacity;
} VM;


void VM_Init();
void VM_Free();
void VM_Push(Value value);
Value VM_Pop();
InterpretResult VM_Interpret(const char *source);

#endif
