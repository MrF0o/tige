//
// Created by fathi on 10/29/2024.
//

#ifndef TIGE_FUNCTIONS_H
#define TIGE_FUNCTIONS_H

#include <stdint.h>
#include "value.h"
#include "bytecode_buffer.h"
#include "memory.h"
#include "uthash.h"

typedef struct TObjectMetadata TObjectMetadata;
typedef struct TObjectProperty TObjectProperty;
typedef struct Function Function;

struct Function {
    TObjectMetadata* metadata;
    TObjectProperty* props;
    BytecodeChunk* chunk;
    char* name;
    Stack* stack;
    size_t arity;
    JumpPlaceholder return_addr;
    UT_hash_handle hh;
};

// TODO: Closures

// Structure representing a call frame
typedef struct CallFrame {
    BytecodeChunk* chunk;
    size_t ip;
    Value* registers;
    struct CallFrame* previous;
} CallFrame;

// Structure representing the call stack
typedef struct CallStack {
    CallFrame* top;
} CallStack;


Function* create_function();
void destroy_function(Function* ptr);
CallStack* create_call_stack();
void destroy_call_stack(CallStack* stack);
bool push_call_frame(CallStack* stack, BytecodeChunk* chunk, size_t ip, Value* registers);
bool pop_call_frame(CallStack* stack, BytecodeChunk** chunk, size_t* ip, Value** registers);

#endif //TIGE_FUNCTIONS_H
