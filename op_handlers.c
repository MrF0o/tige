// op_handlers.c
#include "op_handlers.h"
#include "vm.h"
#include "value.h"
#include "context.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

// TODO: we need a better way to read from memory structured values (eg, uint64_t, double, strings)

// Handler for OP_LOAD_CONST
bool handle_load_const_int(VM *vm) {
    if (vm->ip + sizeof(int32_t) > vm->chunk->size) {
        fprintf(stderr, "Unexpected end of chunk during LOAD_CONST.\n");
        return false;
    }
    int64_t value;
    memcpy(&value, &vm->chunk->bytecode[vm->ip], sizeof(int64_t));
    vm->ip += sizeof(int64_t);
    Value val = make_int(value);
    push(vm, val);
    return true;
}

// Handler for OP_LOAD_STRING
bool handle_load_string(VM *vm) {
    if (vm->ip + sizeof(int32_t) > vm->size) {
        fprintf(stderr, "Unexpected end of bytecode during LOAD_STRING.\n");
        return false;
    }
    int32_t length;
    memcpy(&length, &vm->bytecode[vm->ip], sizeof(int32_t));
    vm->ip += sizeof(int32_t);

    if (vm->ip + length > vm->size) {
        fprintf(stderr, "Unexpected end of bytecode during LOAD_STRING data.\n");
        return false;
    }

    char *chars = (char *) malloc(length + 1);
    if (!chars) {
        fprintf(stderr, "Failed to allocate memory for string characters.\n");
        return false;
    }
    memcpy(chars, &vm->bytecode[vm->ip], length);
    chars[length] = '\0';
    vm->ip += length;

    Value val = make_string(chars);
    push(vm, val);
    return true;
}

// Handler for OP_LOAD_BOOL
bool handle_load_bool(VM *vm) {
    if (vm->ip + sizeof(uint8_t) > vm->chunk->size) {
        fprintf(stderr, "Unexpected end of bytecode during LOAD_BOOL.\n");
        return false;
    }
    uint8_t val = vm->chunk->bytecode[vm->ip++];
    Value bool_val = make_bool(val != 0);
    push(vm, bool_val);
    return true;
}

// Handler for OP_ADD
bool handle_add(VM *vm) {
    if (vm->sp < 1) {
        fprintf(stderr, "Not enough values on stack for ADD operation.\n");
        return false;
    }

    Value b = pop_vm(vm);
    Value a = pop_vm(vm);

    Value result;

    // Integer addition
    if (a.type == VAL_INT && b.type == VAL_INT) {
        result = make_int(a.as_integer + b.as_integer);
    }
        // Float addition
    else if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
        result = make_float(a.as_float + b.as_float);
    } else {
        fprintf(stderr, "ADD operation requires two integers or two floats.\n");
        return false;
    }

    push(vm, result);
    return true;
}

// Handler for OP_SUB
bool handle_sub(VM *vm) {
    if (vm->sp < 1) {
        fprintf(stderr, "Not enough values on stack for SUB operation.\n");
        return false;
    }

    Value a, b;
    b = pop_vm(vm);
    a = pop_vm(vm);

    Value result;

    // if one of them is a float then the result is a float too
    if (a.type == VAL_FLOAT || b.type == VAL_FLOAT) {
        result.type = VAL_FLOAT;
        double operand_a = (a.type == VAL_FLOAT) ? a.as_float : (double) a.as_integer;
        double operand_b = (b.type == VAL_FLOAT) ? b.as_float : (double) b.as_integer;
        result.as_float = operand_a - operand_b;
    } else if (a.type == VAL_INT && b.type == VAL_INT) {
        result.type = VAL_INT;
        result.as_integer = a.as_integer - b.as_integer;
    } else {
        fprintf(stderr, "Error: Unsupported types for SUB operation.\n");
        return false;
    }

    push(vm, result);
    return true;
}

// Handler for OP_MUL
bool handle_mul(VM *vm) {
    if (vm->sp < 1) {
        fprintf(stderr, "Not enough values on stack for SUB operation.\n");
        return false;
    }

    Value a, b;
    b = pop_vm(vm);
    a = pop_vm(vm);

    Value result;

    // if one of them is a float then the result is a float too
    if (a.type == VAL_FLOAT || b.type == VAL_FLOAT) {
        result.type = VAL_FLOAT;
        double operand_a = (a.type == VAL_FLOAT) ? a.as_float : (double) a.as_integer;
        double operand_b = (b.type == VAL_FLOAT) ? b.as_float : (double) b.as_integer;
        result.as_float = operand_a * operand_b;
    } else if (a.type == VAL_INT && b.type == VAL_INT) {
        result.type = VAL_INT;
        result.as_integer = a.as_integer * b.as_integer;
    } else {
        fprintf(stderr, "Error: Unsupported types for MUL operation.\n");
        return false;
    }

    push(vm, result);
    return true;
}

// Handler for OP_DIV
bool handle_div(VM *vm) {
    if (vm->sp < 1) {
        fprintf(stderr, "Not enough values on stack for DIV operugh values on stack for DIV operation.\n");
        return false;
    }

    Value b = pop_vm(vm);
    Value a = pop_vm(vm);

    Value result;

    // Integer division
    if (a.type == VAL_INT && b.type == VAL_INT) {
        if (b.as_integer == 0) {
            fprintf(stderr, "Division by zero!\n");
            return false;
        }
        result = make_int(a.as_integer / b.as_integer);
    }
        // Float division
    else if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
        if (b.as_float == 0.0) {
            fprintf(stderr, "Division by zero!\n");
            return false;
        }
        result = make_float(a.as_float / b.as_float);
    } else {
        fprintf(stderr, "DIV operation requires two integers or two floats.\n");
        return false;
    }

    push(vm, result);
    return true;
}

// Handler for OP_AND
bool handle_and(VM *vm) {
    if (vm->sp < 1) {
        fprintf(stderr, "Not enough values on stack for AND operation.\n");
        return false;
    }

    Value b = pop_vm(vm);
    Value a = pop_vm(vm);

    if (a.type != VAL_BOOL || b.type != VAL_BOOL) {
        fprintf(stderr, "AND operation requires two booleans.\n");
        return false;
    }

    Value result = make_bool(a.as_boolean && b.as_boolean);
    push(vm, result);
    return true;
}

// Handler for OP_OR
bool handle_or(VM *vm) {
    if (vm->sp < 1) {
        fprintf(stderr, "Not enough values on stack for OR operation.\n");
        return false;
    }

    Value b = pop_vm(vm);
    Value a = pop_vm(vm);

    if (a.type != VAL_BOOL || b.type != VAL_BOOL) {
        fprintf(stderr, "OR operation requires two booleans.\n");
        return false;
    }

    Value result = make_bool(a.as_boolean || b.as_boolean);
    push(vm, result);
    return true;
}

// Handler for OP_NOT
bool handle_not(VM *vm) {
    if (vm->sp < 0) {
        fprintf(stderr, "Not enough values on stack for NOT operation.\n");
        return false;
    }

    Value a = pop_vm(vm);

    if (a.type != VAL_BOOL) {
        fprintf(stderr, "NOT operation requires a boolean.\n");
        return false;
    }

    Value result = make_bool(!a.as_boolean);
    push(vm, result);
    return true;
}

// Handler for OP_EQUAL
bool handle_equal(VM *vm) {
    if (vm->sp < 1) {
        fprintf(stderr, "Not enough values on stack for EQUAL operation.\n");
        return false;
    }

    Value b = pop_vm(vm);
    Value a = pop_vm(vm);

    bool is_equal = false;

    if (a.type != b.type) {
        is_equal = false;
    } else {
        switch (a.type) {
            case VAL_INT:
                is_equal = (a.as_integer == b.as_integer);
                break;
            case VAL_FLOAT:
                is_equal = (a.as_float == b.as_float);
                break;
            case VAL_BOOL:
                is_equal = (a.as_boolean == b.as_boolean);
                break;
            case VAL_STRING:
                is_equal = (strcmp(a.as_string, b.as_string) == 0);
                break;
            case VAL_PTR:
                is_equal = (a.as_ptr == b.as_ptr);
                break;
            default:
                is_equal = false;
        }
    }

    Value result = make_bool(is_equal);
    push(vm, result);
    return true;
}

// Handler for OP_NOT_EQUAL
bool handle_not_equal(VM *vm) {
    if (vm->sp < 1) {
        fprintf(stderr, "Not enough values on stack for NOT_EQUAL operation.\n");
        return false;
    }

    Value b = pop_vm(vm);
    Value a = pop_vm(vm);

    bool is_not_equal = false;

    if (a.type != b.type) {
        is_not_equal = true;
    } else {
        switch (a.type) {
            case VAL_INT:
                is_not_equal = (a.as_integer != b.as_integer);
                break;
            case VAL_FLOAT:
                is_not_equal = (a.as_float != b.as_float);
                break;
            case VAL_BOOL:
                is_not_equal = (a.as_boolean != b.as_boolean);
                break;
            case VAL_STRING:
                is_not_equal = (strcmp(a.as_string, b.as_string) != 0);
                break;
            case VAL_PTR:
                is_not_equal = (a.as_ptr != b.as_ptr);
                break;
            default:
                is_not_equal = true;
        }
    }

    Value result = make_bool(is_not_equal);
    push(vm, result);
    return true;
}

// Handler for OP_LESS_THAN
bool handle_less_than(VM *vm) {
    if (vm->sp < 1) {
        fprintf(stderr, "Not enough values on stack for LESS_THAN operation.\n");
        return false;
    }

    Value b = pop_vm(vm);
    Value a = pop_vm(vm);

    bool result_bool = false;

    // Integer comparison
    if (a.type == VAL_INT && b.type == VAL_INT) {
        result_bool = (a.as_integer < b.as_integer);
    }
        // Float comparison
    else if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
        result_bool = (a.as_float < b.as_float);
    } else {
        fprintf(stderr, "LESS_THAN operation requires two integers or two floats.\n");
        return false;
    }

    Value result = make_bool(result_bool);
    push(vm, result);
    return true;
}

// Handler for OP_GREATER_THAN
bool handle_greater_than(VM *vm) {
    if (vm->sp < 1) {
        fprintf(stderr, "Not enough values on stack for GREATER_THAN operation.\n");
        return false;
    }

    Value b = pop_vm(vm);
    Value a = pop_vm(vm);

    bool result_bool = false;

    // Integer comparison
    if (a.type == VAL_INT && b.type == VAL_INT) {
        result_bool = (a.as_integer > b.as_integer);
    }
        // Float comparison
    else if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
        result_bool = (a.as_float > b.as_float);
    } else {
        fprintf(stderr, "GREATER_THAN operation requires two integers or two floats.\n");
        return false;
    }

    Value result = make_bool(result_bool);
    push(vm, result);
    return true;
}

// Handler for OP_LESS_EQUAL
bool handle_less_equal(VM *vm) {
    if (vm->sp < 1) {
        fprintf(stderr, "Not enough values on stack for LESS_EQUAL operation.\n");
        return false;
    }

    Value b = pop_vm(vm);
    Value a = pop_vm(vm);

    bool result_bool = false;

    // Integer comparison
    if (a.type == VAL_INT && b.type == VAL_INT) {
        result_bool = (a.as_integer <= b.as_integer);
    }
        // Float comparison
    else if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
        result_bool = (a.as_float <= b.as_float);
    } else {
        fprintf(stderr, "LESS_EQUAL operation requires two integers or two floats.\n");
        return false;
    }

    Value result = make_bool(result_bool);
    push(vm, result);
    return true;
}

// Handler for OP_GREATER_EQUAL
bool handle_greater_equal(VM *vm) {
    if (vm->sp < 1) {
        fprintf(stderr, "Not enough values on stack for GREATER_EQUAL operation.\n");
        return false;
    }

    Value b = pop_vm(vm);
    Value a = pop_vm(vm);

    bool result_bool = false;

    // Integer comparison
    if (a.type == VAL_INT && b.type == VAL_INT) {
        result_bool = (a.as_integer >= b.as_integer);
    }
        // Float comparison
    else if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
        result_bool = (a.as_float >= b.as_float);
    } else {
        fprintf(stderr, "GREATER_EQUAL operation requires two integers or two floats.\n");
        return false;
    }

    Value result = make_bool(result_bool);
    push(vm, result);
    return true;
}

// Handler for OP_JMP
bool handle_jmp(VM *vm) {
    if (vm->ip + sizeof(size_t) * 2 > vm->chunk->size) {
        fprintf(stderr, "Unexpected end of chunk during JMP.\n");
        return false;
    }

    size_t chunk_id = vm_read_offset(vm);
    size_t jmp_offset = vm_read_offset(vm);


    if (vm_jump_to_chunk(vm, chunk_id)) {
        vm->ip = jmp_offset;
    } else {
        return false;
    }
    return true;
}

// jump to a region in memory that contains a valid BytecodeChunk
bool handle_jmp_adr(VM *vm) {
    uintptr_t jmp_to = vm_read_ptr(vm);
    // get the chunk addr
    BytecodeChunk *chunk = (BytecodeChunk *) (jmp_to);
    vm->chunk = chunk;
    vm->ip = 0;

    return true;
}

// Handler for OP_JMP_IF_TRUE
bool handle_jmp_if_true(VM *vm) {
    if (vm->ip + sizeof(size_t) * 2 > vm->chunk->size) {
        fprintf(stderr, "Unexpected end of chunk during JMP_IF_TRUE.\n");
        return false;
    }

    size_t chunk_id = vm_read_offset(vm);
    size_t jmp_offset = vm_read_offset(vm);

    if (vm->sp < 0) {
        fprintf(stderr, "Not enough values on stack for JMP_IF_TRUE.\n");
        return false;
    }
    Value condition = pop_vm(vm);
    if (condition.type != VAL_BOOL) {
        fprintf(stderr, "JMP_IF_TRUE requires a boolean condition.\n");
        return false;
    }
    if (condition.as_boolean) {
        if (vm_jump_to_chunk(vm, chunk_id)) {
            vm->ip = jmp_offset;
        } else {
            return false;
        }
    }
    return true;
}

// Handler for OP_JMP_IF_FALSE
bool handle_jmp_if_false(VM *vm) {
    if (vm->ip + sizeof(size_t) * 2 > vm->chunk->size) {
        fprintf(stderr, "Unexpected end of chunk during JMP_IF_FALSE.\n");
        return false;
    }

    size_t chunk_id = vm_read_offset(vm);
    size_t jmp_offset = vm_read_offset(vm);

    if (vm->sp < 0) {
        fprintf(stderr, "Not enough values on stack for JMP_IF_FALSE.\n");
        return false;
    }
    Value condition = pop_vm(vm);
    if (condition.type != VAL_BOOL) {
        fprintf(stderr, "JMP_IF_FALSE requires a boolean condition.\n");
        return false;
    }
    if (!condition.as_boolean) {
        if (vm_jump_to_chunk(vm, chunk_id)) {
            vm->ip = jmp_offset;
        } else {
            return false;
        }
    }
    return true;
}

// Handler for OP_CALL
bool handle_call(VM *vm) {
    if (vm->ip + sizeof(int32_t) > vm->size) {
        fprintf(stderr, "Unexpected end of bytecode during CALL.\n");
        return false;
    }
    int32_t func_address;
    memcpy(&func_address, &vm->bytecode[vm->ip], sizeof(int32_t));
    vm->ip += sizeof(int32_t);
    if (func_address < 0 || (size_t) func_address >= vm->size) {
        fprintf(stderr, "CALL to invalid function address: %d\n", func_address);
        return false;
    }
    // Push current ip as return address (stored as VAL_INT)
    Value return_address = make_int(vm->ip);
    push(vm, return_address);
    // Jump to function address
    vm->ip = func_address;
    return true;
}

// Handler for OP_RETURN
bool handle_return(VM *vm) {
    if (vm->sp < 0) {
        fprintf(stderr, "Nothing on stack to return.\n");
        return false;
    }
    Value ret = pop_vm(vm);
    push(vm, ret);
    return true;
    // Pop return address
    if (vm->sp < 0) {
        fprintf(stderr, "No return address on stack.\n");
        return false;
    }
    Value return_address_val = pop_vm(vm);
    if (return_address_val.type != VAL_INT) {
        fprintf(stderr, "RETURN expects an integer return address.\n");
        return false;
    }
    int32_t return_address = return_address_val.as_integer;
    if (return_address < 0 || (size_t) return_address >= vm->size) {
        fprintf(stderr, "RETURN to invalid address: %d\n", return_address);
        return false;
    }
    vm->ip = return_address;
    // Optionally, push the return value back onto the stack
    push(vm, ret);
    return true;
}

// Handler for OP_NEW_OBJECT
bool handle_new_object(VM *vm) {
    // TODO: Implement object creation
    fprintf(stderr, "OP_NEW_OBJECT not implemented.\n");
    return false;
    return false; // Unreachable
}

// Handler for OP_GET_PROPERTY
bool handle_get_property(VM *vm) {
    // TODO: Implement property retrieval
    fprintf(stderr, "OP_GET_PROPERTY not implemented.\n");
    return false;
    return false; // Unreachable
}

// Handler for OP_SET_PROPERTY
bool handle_set_property(VM *vm) {
    // TODO: Implement property setting
    fprintf(stderr, "OP_SET_PROPERTY not implemented.\n");
    return false;
    return false; // Unreachable
}

// Handler for OP_ALLOC_HEAP
bool handle_alloc_heap(VM *vm) {
    // TODO: Implement heap allocation
    fprintf(stderr, "OP_ALLOC_HEAP not implemented.\n");
    return false;
    return false; // Unreachable
}

// Handler for OP_FREE_HEAP
bool handle_free_heap(VM *vm) {
    // TODO: Implement heap freeing
    fprintf(stderr, "OP_FREE_HEAP not implemented.\n");
    return false;
    return false; // Unreachable
}

// Handler for OP_TERNARY
bool handle_ternary(VM *vm) {
    if (vm->sp < 2) {
        fprintf(stderr, "Not enough values on stack for TERNARY operation.\n");
        return false;
    }

    Value false_val = pop_vm(vm);
    Value true_val = pop_vm(vm);
    Value condition = pop_vm(vm);

    if (condition.type != VAL_BOOL) {
        fprintf(stderr, "TERNARY operation requires a boolean condition.\n");
        return false;
    }

    Value result = condition.as_boolean ? true_val : false_val;
    push(vm, result);
    return true;
}

// Handler for OP_HALT
bool handle_halt(VM *vm) {
    if (vm->sp < 0) {
        fprintf(stderr, "Nothing on stack to return.\n");
        return false;
    }
    Value ret = pop_vm(vm);
    if (ret.type == VAL_INT) {
        printf("VM Halt with exit code: %lld\n", ret.as_integer);
    } else {
        fprintf(stderr, "HALT expects an integer on the stack.\n");
        return false;
    }
    return false; // Unreachable
}

// Handler for OP_NOP
bool handle_nop(VM *vm) {
    // No operation; simply continue execution
    return true;
}

// Handler for OP_STORE_VAR
// OP_STORE_VAR <index:uint64_t>
// where index is the index of the variable in the symbol table
bool handle_store_var(VM *vm) {
    uint16_t variable_index = vm_read_uint16(vm);
    Value val = pop_vm(vm);
    vm->registers[variable_index] = val;
    return true;
}

bool handle_load_const_float(VM *vm) {
    if (vm->ip + sizeof(double) > vm->size) {
        fprintf(stderr, "Unexpected end of bytecode during LOAD_CONST.\n");
        return false;
    }

    double value;
    memcpy(&value, &vm->bytecode[vm->ip], sizeof(double));
    vm->ip += sizeof(double);
    Value val = make_float(value);
    push(vm, val);
    return true;
}

bool handle_load_var(VM *vm) {
    uint16_t variable_index = vm_read_uint16(vm);
    push(vm, vm->registers[variable_index]);
    return true;
}

bool handle_enter_scope(VM *vm) {
    if (vm->context->symbols) {
        enter_scope(vm->context->symbols);
    } else {
        vm->context->symbols = create_symbol_table();
    }

    return true;
}

bool handle_exit_scope(VM *vm) {
    exit_scope(vm->context->symbols);
    return true;
}

bool handle_pop(VM *vm) {
    pop_vm(vm);
    return true;
}

bool handle_push(VM *vm) {
    return false;
}

bool handle_save_sp(VM *vm) {
    vm->sp_reset = vm->sp;
    return true;
}

bool handle_reset_sp(VM *vm) {
    // for (int i = vm->sp; i <= vm->sp_reset; i--) pop_vm(vm);
    vm->sp = vm->sp_reset;
    return true;
}

bool handle_inc_reg(VM *vm) {
    uint16_t variable_index = vm_read_uint16(vm);
    Value val = vm->registers[variable_index];

    if (val.type == VAL_INT) {
        ((vm->registers) + variable_index)->as_integer++;
    }

    return true;
}

