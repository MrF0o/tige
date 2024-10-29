//
// Created by fathi on 10/29/2024.
//

#ifndef TIGE_FUNCTIONS_H
#define TIGE_FUNCTIONS_H

#include <stdint.h>
#include "value.h"
#include "object.h"
#include "bytecode_buffer.h"

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

// Function prototypes
CallStack* create_call_stack();
void destroy_call_stack(CallStack* stack);
bool push_call_frame(CallStack* stack, BytecodeChunk* chunk, size_t ip, Value* registers);
bool pop_call_frame(CallStack* stack, BytecodeChunk** chunk, size_t* ip, Value** registers);

#endif //TIGE_FUNCTIONS_H
