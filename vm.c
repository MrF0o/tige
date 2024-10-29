//
// Created by fathi on 10/21/2024.
//

#include "vm.h"
#include "op_handlers.h"
#include "context.h"
#include "bytecode_buffer.h"
#include <stdio.h>
#include <memory.h>

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

        [OP_CALL]            = nullptr,                // 0x14 (to be implemented)
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
        // All other opcodes remain NULL by default
};

// Initialize the VM
VM *create_vm(Context *context) {
    VM *vm = (VM *) malloc(sizeof(VM));
    // TODO: remove these two lines
    vm->bytecode = context->code->head->bytecode;
    vm->size = context->code->head->size;

    vm->buffer = context->code;
    vm->chunk = context->code->head;
    vm->context = context;
    vm->ip = 0;
    vm->sp = -1; // Empty stack
    return vm;
}

// Destroy the VM
void destroy_vm(VM *vm) {
    if (vm) {
        free(vm);
    }
}

// Push a value onto the stack
void push(VM *vm, Value value) {
    if (vm->sp >= STACK_SIZE) {
        fprintf(stderr, "Stack overflow!\n");
        exit(1);
    }
    vm->stack[++vm->sp] = value;
}

// Pop a value from the stack
Value pop_vm(VM *vm) {
    if (vm->sp < 0) {
        fprintf(stderr, "Stack underflow!\n");
        exit(1);
    }
    auto val = vm->stack[vm->sp];
    vm->stack[vm->sp--] = make_null();
    return val;
}

Value vm_execute(VM *vm) {
    while (vm->ip < vm->size) {
        // total num of chunks
        auto op = (Opcode) vm->chunk->bytecode[vm->ip];
        vm->ip++;
        auto handler = opcode_handlers[op];
        bool continue_execution = handler(vm);

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

    if (vm->sp >= 0) {
        return pop_vm(vm);
    }

    return make_null();
}

char *vm_read_string(VM *vm) {
    if (vm == NULL) {
        fprintf(stderr, "Error: vm_read_string received NULL VM pointer.\n");
        return nullptr;
    }

    if (vm->chunk->bytecode == NULL) {
        fprintf(stderr, "Error: vm_read_string received NULL bytecode pointer.\n");
        return nullptr;
    }

    if (vm->ip >= vm->chunk->size) {
        fprintf(stderr, "Error: Instruction pointer (%zu) out of bounds of the current chunk while reading string.\n",
                vm->ip);
        exit(-1);
    }

    size_t remaining = vm->chunk->size - vm->ip;

    void *null_pos = memchr(vm->chunk->bytecode + vm->ip, '\0', remaining);
    if (null_pos == NULL) {
        fprintf(stderr, "Error: Null terminator not found for string starting at position %zu.\n", vm->ip);
        exit(-1);
    }

    size_t str_length = (uint8_t *) null_pos - (vm->chunk->bytecode + vm->ip);

    char *string = (char *) malloc(str_length + 1);
    if (string == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for string at position %zu.\n", vm->ip);
        return nullptr;
    }

    memcpy(string, vm->chunk->bytecode + vm->ip, str_length);

    string[str_length] = '\0';
    vm->ip += str_length + 1;

    return string;
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
