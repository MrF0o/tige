//
// Created by fathi on 10/29/2024.
//

#include "functions.h"
#include <stdlib.h>
#include <stdio.h>

// Create a new function
Function* create_function() {
    auto fn = (Function*)vm_malloc(sizeof(Function));
    if (!fn) {
        fprintf(stderr, "Failed to allocate memory for function");
        exit(EXIT_FAILURE);
    }

    // we will need to fill up these info whenever we create a new function
    fn->chunk = nullptr;
    fn->arity = 0;
    fn->stack = nullptr;
    return fn;
}

// Create a new call stack
CallStack* create_call_stack() {
    CallStack* stack = malloc(sizeof(CallStack));
    if (!stack) {
        fprintf(stderr, "Failed to allocate CallStack.\n");
        exit(1);
    }
    stack->top = nullptr;
    return stack;
}

// Destroy the call stack
void destroy_call_stack(CallStack* stack) {
    while (stack->top) {
        CallFrame* frame = stack->top;
        stack->top = frame->previous;
        // Assuming registers are allocated dynamically per frame
        vm_free(frame->registers);
        vm_free(frame);
    }
    free(stack);
}

// Push a new call frame onto the stack
bool push_call_frame(CallStack* stack, BytecodeChunk* chunk, size_t ip, Value* registers) {
    CallFrame* frame = vm_malloc(sizeof(CallFrame));
    if (!frame) {
        fprintf(stderr, "Failed to allocate CallFrame.\n");
        return false;
    }
    frame->chunk = chunk;
    frame->ip = ip;
    frame->registers = registers;
    frame->previous = stack->top;
    stack->top = frame;
    return true;
}

// Pop the top call frame from the stack
bool pop_call_frame(CallStack* stack, BytecodeChunk** chunk, size_t* ip, Value** registers) {
    if (!stack->top) {
        fprintf(stderr, "Call stack underflow!\n");
        return false;
    }
    CallFrame* frame = stack->top;
    stack->top = frame->previous;
    *chunk = frame->chunk;
    *ip = frame->ip;
    *registers = frame->registers;
    vm_free(frame);
    return true;
}

void destroy_function(Function *ptr) {
    free(ptr->name);
    object_free((void*)ptr);
}
