#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "value.h"
#include "vm.h"


VM vm;

static void VM_StackInit();
/* static InterpretResult run(); */


void VM_Init()
{
    VM_StackInit();
}


static void reset_stack()
{
    vm.stack_top = vm.stack;
}

static void VM_StackInit()
{
    vm.stack = NULL;
    vm.stack_top = NULL;
    vm.stack_capacity = 0;
}


static void VM_StackFree()
{
    FREE_ARRAY(Value, vm.stack, vm.stack_capacity);
    VM_StackInit();
}


void VM_Free()
{
}


void VM_Push(Value value)
{
    int count, old_capacity;

    count = (vm.stack_top == NULL) ? 0 : vm.stack_top - vm.stack;
    if (vm.stack_capacity < (count + 1)) {
        old_capacity = vm.stack_capacity;
        vm.stack_capacity = GROW_CAPACITY(old_capacity);
        vm.stack = GROW_ARRAY(Value, vm.stack, old_capacity, vm.stack_capacity);
        vm.stack_top = vm.stack + count;
    }

    *vm.stack_top++ = value;
}


Value VM_Pop()
{
    int count, old_capacity;

    vm.stack_top--;

    count = vm.stack_top - vm.stack;
    if (((float)count / (float)vm.stack_capacity) < 0.25) {
        old_capacity = vm.stack_capacity;
        vm.stack_capacity = GROW_CAPACITY(vm.stack_capacity / 2);
        vm.stack = GROW_ARRAY(Value, vm.stack, old_capacity, vm.stack_capacity);
        vm.stack_top = vm.stack + count;
    }

    return *vm.stack_top;
}


InterpretResult VM_Interpret(const char *source)
{
    compile(source);
    return INTERPRET_OK;
}


InterpretResult run(Chunk *chunk)
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_CONSTANT_LONG() \
    (vm.chunk->constants.values[READ_BYTE() | (READ_BYTE() << 8) | (READ_BYTE() << 16)])
#define BINARY_OP(op) \
    do { \
        Value b = VM_Pop(); \
        Value a = VM_Pop(); \
        VM_Push(a op b); \
    } while (false)

    vm.chunk = chunk;
    vm.ip = vm.chunk->code;

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value *slot = vm.stack; slot < vm.stack_top; slot++) {
            printf("[");
            Value_Print(*slot);
            printf("]");
        }
        printf("\n");
        Chunk_DisassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));    
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                VM_Push(constant);
                break;
            }
            case OP_CONSTANT_LONG: {
                Value constant = READ_CONSTANT_LONG();
                VM_Push(constant);
                break;
            }
            case OP_ADD: {
                BINARY_OP(+);
                break;
            }
            case OP_SUBSTRACT: {
                BINARY_OP(-);
                break;
            }
            case OP_MULTIPLY: {
                BINARY_OP(*);
                break;
            }
            case OP_DIVIDE: {
                BINARY_OP(/);
                break;
            }
            case OP_NEGATE: {
                VM_Push(-VM_Pop());
                break;
            }
            case OP_RETURN: {
                Value_Print(VM_Pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }
    return INTERPRET_RUNTIME_ERROR;

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

