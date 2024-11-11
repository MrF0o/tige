//
// Created by fathi on 10/21/2024.
//

#include "vm.h"
#include "op_handlers.h"
#include "context.h"
#include "bytecode_buffer.h"
#include <stdio.h>
#include <memory.h>

static VM *g_vm = nullptr;

static OpcodeHandler opcode_handlers[256] = {
        [OP_NOPE]            = handle_nop,             // 0x00
        [OP_LOAD_CONST_INT]  = handle_load_const_int,  // 0x01
        [OP_LOAD_CONST_FLOAT]= handle_load_const_float,
        [OP_LOAD_VAR]        = handle_load_var,        // 0x02
        [OP_STORE_VAR]       = handle_store_var,       // 0x03

        [OP_ADD]             = handle_add,             // 0x04
        [OP_SUB]             = handle_sub,             // 0x05
        [OP_MUL]             = handle_mul,             // 0x06
        [OP_DIV]             = handle_div,             // 0x07

        [OP_AND]             = handle_and,             // 0x08 (to be implemented)
        [OP_OR]              = handle_or,              // 0x09 (to be implemented)
        [OP_NOT]             = handle_not,             // 0x0A (to be implemented)

        [OP_EQUAL]           = handle_equal,           // 0x0B (to be implemented)
        [OP_NOT_EQUAL]       = handle_not_equal,       // 0x0C (to be implemented)
        [OP_LESS_THAN]       = handle_less_than,       // 0x0D (to be implemented)
        [OP_GREATER_THAN]    = handle_greater_than,    // 0x0E (to be implemented)
        [OP_LESS_EQUAL]      = handle_less_equal,      // 0x0F (to be implemented)
        [OP_GREATER_EQUAL]   = handle_greater_equal,   // 0x10 (to be implemented)

        [OP_JMP]             = handle_jmp,                // 0x11 (to be implemented)
        [OP_JMP_IF_TRUE]     = handle_jmp_if_true,        // 0x12 (to be implemented)
        [OP_JMP_IF_FALSE]    = handle_jmp_if_false,       // 0x13 (to be implemented)

        [OP_CALL]            = handle_call,                // 0x14 (to be implemented)
        [OP_RETURN]          = handle_return,          // 0x15

        [OP_NEW_OBJECT]      = nullptr,                // 0x16 (to be implemented)
        [OP_GET_PROPERTY]    = nullptr,                // 0x17 (to be implemented)
        [OP_SET_PROPERTY]    = nullptr,                // 0x18 (to be implemented)

        [OP_ALLOC_HEAP]      = nullptr,                // 0x19 (to be implemented)
        [OP_FREE_HEAP]       = nullptr,                // 0x1A (to be implemented)

        [OP_LOAD_STRING]     = handle_load_string,     // 0x1B
        [OP_LOAD_BOOL]       = handle_load_bool,       // 0x1C

        [OP_TERNARY]         = nullptr,                // 0x1D

        [OP_JMP_ADR]         = handle_jmp_adr,

        [OP_ENTER_SCOPE]     = handle_enter_scope,
        [OP_EXIT_SCOPE]      = handle_exit_scope,

        [OP_PUSH]            = handle_push,
        [OP_POP]             = handle_pop,

        [OP_SAVE_SP]         = handle_save_sp,
        [OP_RESET_SP]        = handle_reset_sp,

        [OP_INC_REG]         = handle_inc_reg,

        [OP_HALT]            = handle_halt,            // 0xFF
        // All other opcodes remain nullptr by default
};

// Initialize the VM
VM *create_vm(Context *context) {
    VM *vm = (VM *) malloc(sizeof(VM));
    // TODO: remove these two lines
    vm->bytecode = nullptr;
    vm->size = -1;

    vm->buffer = context->code;
    vm->chunk = nullptr;
    vm->context = context;
    vm->call_stack = create_call_stack();
    vm->ip = 0;
    vm->sp = -1; // Empty stack

    for (int i = 0; i < MAX_REGISTERS; i++) {
        vm->registers[i] = make_null(); // TODO: change to make_undefined
    }

    vm->stack = create_stack(STACK_SIZE);
    vm->heap = create_heap();

    // TODO: GC thread here

    g_vm = vm;
    return vm;
}

// Destroy the VM
void destroy_vm(VM *vm) {
    if (vm) {
        free(vm);
        g_vm = nullptr;
    }
}

// TODO: remove the vm argument and use get_vm()
// Push a value onto the stack
inline void vm_push(VM *vm, Value value) {
    if (!g_vm) {
        fprintf(stderr, "VM not initialized.\n");
        // TODO: handle error
        exit(EXIT_FAILURE);
    }

    bool success = push_stack(g_vm->stack, value);
    // TODO: error handling for non-successful stack operation
}

// TODO: remove the vm argument and use get_vm()
// Pop a value from the stack
inline Value vm_pop(VM *vm) {
    if (!g_vm) {
        fprintf(stderr, "VM not initialized.\n");
        // TODO: handle error
        exit(EXIT_FAILURE);
    }

    Value val;
    bool success = pop_stack(g_vm->stack, &val);
    // TODO: error handling for non-successful stack operation
    return val;
}

Value vm_execute(VM *vm) {
    while (vm->ip < vm->size) {
        // total num of chunks
        auto op = (Opcode) vm->chunk->bytecode[vm->ip];
        vm->ip++;
        auto handler = opcode_handlers[op];
        bool continue_execution = handler();

        if (!continue_execution) {
            break;
        } else {
            // this is temp code to be removed
            if (vm->ip >= vm->chunk->size) {
                vm->ip = 0;
            } else {
                continue; // each handler increments the ip with the required size
            }
        }
    }

    if (SP >= 0) {
        return vm_pop(vm);
    }

    return make_null();
}

// read a TString object pointer from the bytecode
TString *vm_read_string(VM *vm) {
    if (vm == nullptr) {
        fprintf(stderr, "Error: vm_read_string received nullptr VM pointer.\n");
        return nullptr;
    }

    if (vm->chunk->bytecode == nullptr) {
        fprintf(stderr, "Error: vm_read_string received nullptr bytecode pointer.\n");
        return nullptr;
    }

    if (vm->ip >= vm->chunk->size) {
        fprintf(stderr, "Error: Instruction pointer (%zu) out of bounds of the current chunk while reading string.\n",
                vm->ip);
        exit(-1);
    }

    const auto str = (TString*)vm_read_ptr(vm);


    return str;
}

// read a plain string from the current bytecode location
inline const char* vm_read_fn_name(VM* vm)
{
    if (vm == nullptr) {
        fprintf(stderr, "Error: vm_read_fn_name received nullptr VM pointer.\n");
        return nullptr;
    }

    if (vm->chunk == nullptr || vm->chunk->bytecode == nullptr) {
        fprintf(stderr, "Error: vm_read_fn_name received nullptr chunk or bytecode pointer.\n");
        return nullptr;
    }

    if (vm->ip >= vm->chunk->size) {
        fprintf(stderr, "Error: Instruction pointer (%zu) out of bounds of the current chunk while reading function name.\n",
                vm->ip);
        exit(EXIT_FAILURE);
    }

    size_t remaining = vm->chunk->size - vm->ip;

    // Find the null terminator within the remaining bytes
    void* null_pos = memchr(vm->chunk->bytecode + vm->ip, '\0', remaining);
    if (null_pos == nullptr) {
        fprintf(stderr, "Error: Null terminator not found for function name starting at position %zu.\n", vm->ip);
        exit(EXIT_FAILURE);
    }

    // Calculate the length of the string
    size_t str_length = (uint8_t*)null_pos - (vm->chunk->bytecode + vm->ip);

    // Get the pointer to the function name in the bytecode
    const char* name = (const char*)(vm->chunk->bytecode + vm->ip);

    // Move the instruction pointer past the string and its null terminator
    vm->ip += str_length + 1;

    return name;
}

uintptr_t vm_read_ptr(VM *vm) {
    return vm_read_uint(vm);
}

uint64_t vm_read_uint(VM *vm) {
    uint64_t value;
    memcpy(&value, vm->chunk->bytecode + vm->ip, sizeof(uint64_t));
    vm->ip += sizeof(uint64_t);
    return value;
}

size_t vm_read_offset(VM *vm) {
    size_t value;
    memcpy(&value, vm->chunk->bytecode + vm->ip, sizeof(size_t));
    vm->ip += sizeof(size_t);
    return value;
}

bool vm_jump_to_chunk(VM *vm, size_t chunk_id) {
    auto chunk = bc_get_chunk_by_id(vm->buffer, chunk_id);
    if (chunk) {
        vm->chunk = chunk;
        return true;
    }
    return false;
}

uint16_t vm_read_uint16(VM *vm) {
    uint16_t value;
    memcpy(&value, vm->chunk->bytecode + vm->ip, sizeof(uint16_t));
    vm->ip += sizeof(uint16_t);
    return value;
}

inline VM* get_vm(void) {
    return g_vm;
}
