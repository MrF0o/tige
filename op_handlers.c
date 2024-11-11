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
inline bool handle_load_const_int(void) {
    auto vm = get_vm();
    if (vm->ip + sizeof(int64_t) > vm->chunk->size) {
        fprintf(stderr, "Unexpected end of chunk during LOAD_CONST.\n");
        return false;
    }
    int64_t value;
    memcpy(&value, &vm->chunk->bytecode[vm->ip], sizeof(int64_t));
    vm->ip += sizeof(int64_t);
    Value val = make_int(value);
    vm_push(vm, val);
    return true;
}

// Handler for OP_LOAD_STRING
inline bool handle_load_string(void) {
    const auto vm = get_vm();
    const auto str = vm_read_string(vm);
    const Value val = make_string(str->chars);
    vm_push(vm, val);

    // TODO: GC String

    return true;
}

// Handler for OP_LOAD_BOOL
inline bool handle_load_bool(void) {
    auto vm = get_vm();
    if (vm->ip + sizeof(uint8_t) > vm->chunk->size) {
        fprintf(stderr, "Unexpected end of bytecode during LOAD_BOOL.\n");
        return false;
    }
    uint8_t val = vm->chunk->bytecode[vm->ip++];
    Value bool_val = make_bool(val != 0);
    vm_push(vm, bool_val);
    return true;
}

// Handler for OP_ADD
inline bool handle_add(void) {
    auto vm = get_vm();

    if (SP < 1) {
        fprintf(stderr, "Not enough values on stack for ADD operation.\n");
        return false;
    }

    Value b = vm_pop(vm);
    Value a = vm_pop(vm);

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

    vm_push(vm, result);
    return true;
}

// Handler for OP_SUB
inline bool handle_sub(void) {
    auto vm = get_vm();

    if (SP < 1) {
        fprintf(stderr, "Not enough values on stack for SUB operation.\n");
        return false;
    }

    Value a, b;
    b = vm_pop(vm);
    a = vm_pop(vm);

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

    vm_push(vm, result);
    return true;
}

// Handler for OP_MUL
inline bool handle_mul(void) {
    auto vm = get_vm();
    if (SP < 1) {
        fprintf(stderr, "Not enough values on stack for SUB operation.\n");
        return false;
    }

    Value a, b;
    b = vm_pop(vm);
    a = vm_pop(vm);

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

    vm_push(vm, result);
    return true;
}

// Handler for OP_DIV
inline bool handle_div(void) {
    auto vm = get_vm();
    if (SP < 1) {
        fprintf(stderr, "Not enough values on stack for DIV operugh values on stack for DIV operation.\n");
        return false;
    }

    Value b = vm_pop(vm);
    Value a = vm_pop(vm);

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

    vm_push(vm, result);
    return true;
}

// Handler for OP_AND
inline bool handle_and(void) {
    auto vm = get_vm();
    if (SP < 1) {
        fprintf(stderr, "Not enough values on stack for AND operation.\n");
        return false;
    }

    Value b = vm_pop(vm);
    Value a = vm_pop(vm);

    if (a.type != VAL_BOOL || b.type != VAL_BOOL) {
        fprintf(stderr, "AND operation requires two booleans.\n");
        return false;
    }

    Value result = make_bool(a.as_boolean && b.as_boolean);
    vm_push(vm, result);
    return true;
}

// Handler for OP_OR
inline bool handle_or(void) {
    auto vm = get_vm();
    if (SP < 1) {
        fprintf(stderr, "Not enough values on stack for OR operation.\n");
        return false;
    }

    Value b = vm_pop(vm);
    Value a = vm_pop(vm);

    if (a.type != VAL_BOOL || b.type != VAL_BOOL) {
        fprintf(stderr, "OR operation requires two booleans.\n");
        return false;
    }

    Value result = make_bool(a.as_boolean || b.as_boolean);
    vm_push(vm, result);
    return true;
}

// Handler for OP_NOT
bool handle_not(void) {
    auto vm = get_vm();
    if (SP < 0) {
        fprintf(stderr, "Not enough values on stack for NOT operation.\n");
        return false;
    }

    Value a = vm_pop(vm);

    if (a.type != VAL_BOOL) {
        fprintf(stderr, "NOT operation requires a boolean.\n");
        return false;
    }

    Value result = make_bool(!a.as_boolean);
    vm_push(vm, result);
    return true;
}

// Handler for OP_EQUAL
inline bool handle_equal(void) {
    auto vm = get_vm();

    if (SP < 1) {
        fprintf(stderr, "Not enough values on stack for EQUAL operation.\n");
        return false;
    }

    Value b = vm_pop(vm);
    Value a = vm_pop(vm);

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
    vm_push(vm, result);
    return true;
}

// Handler for OP_NOT_EQUAL
bool handle_not_equal(void) {
    auto vm = get_vm();
    if (SP < 1) {
        fprintf(stderr, "Not enough values on stack for NOT_EQUAL operation.\n");
        return false;
    }

    Value b = vm_pop(vm);
    Value a = vm_pop(vm);

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
    vm_push(vm, result);
    return true;
}

// Handler for OP_LESS_THAN
bool handle_less_than(void) {
    auto vm = get_vm();
    if (SP < 1) {
        fprintf(stderr, "Not enough values on stack for LESS_THAN operation.\n");
        return false;
    }

    Value b = vm_pop(vm);
    Value a = vm_pop(vm);

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
    vm_push(vm, result);
    return true;
}

// Handler for OP_GREATER_THAN
bool handle_greater_than(void) {
    auto vm = get_vm();
    if (SP < 1) {
        fprintf(stderr, "Not enough values on stack for GREATER_THAN operation.\n");
        return false;
    }

    Value b = vm_pop(vm);
    Value a = vm_pop(vm);

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
    vm_push(vm, result);
    return true;
}

// Handler for OP_LESS_EQUAL
bool handle_less_equal(void) {
    auto vm = get_vm();
    if (SP < 1) {
        fprintf(stderr, "Not enough values on stack for LESS_EQUAL operation.\n");
        return false;
    }

    Value b = vm_pop(vm);
    Value a = vm_pop(vm);

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
    vm_push(vm, result);
    return true;
}

// Handler for OP_GREATER_EQUAL
bool handle_greater_equal(void) {
    auto vm = get_vm();
    if (SP < 1) {
        fprintf(stderr, "Not enough values on stack for GREATER_EQUAL operation.\n");
        return false;
    }

    Value b = vm_pop(vm);
    Value a = vm_pop(vm);

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
    vm_push(vm, result);
    return true;
}

// Handler for OP_JMP
inline bool handle_jmp(void) {
    auto vm = get_vm();
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
inline bool handle_jmp_adr(void) {
    auto vm = get_vm();
    uintptr_t jmp_to = vm_read_ptr(vm);
    // get the chunk addr
    BytecodeChunk *chunk = (BytecodeChunk *) (jmp_to);
    vm->chunk = chunk;
    vm->ip = 0;

    return true;
}

// Handler for OP_JMP_IF_TRUE
inline bool handle_jmp_if_true(void) {
    auto vm = get_vm();
    if (vm->ip + sizeof(size_t) * 2 > vm->chunk->size) {
        fprintf(stderr, "Unexpected end of chunk during JMP_IF_TRUE.\n");
        return false;
    }

    size_t chunk_id = vm_read_offset(vm);
    size_t jmp_offset = vm_read_offset(vm);

    if (SP < 0) {
        fprintf(stderr, "Not enough values on stack for JMP_IF_TRUE.\n");
        return false;
    }
    Value condition = vm_pop(vm);
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
inline bool handle_jmp_if_false(void) {
    auto vm = get_vm();
    if (vm->ip + sizeof(size_t) * 2 > vm->chunk->size) {
        fprintf(stderr, "Unexpected end of chunk during JMP_IF_FALSE.\n");
        return false;
    }

    size_t chunk_id = vm_read_offset(vm);
    size_t jmp_offset = vm_read_offset(vm);

    if (SP < 0) {
        fprintf(stderr, "Not enough values on stack for JMP_IF_FALSE.\n");
        return false;
    }
    Value condition = vm_pop(vm);
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

void std_out(VM* vm)
{
    const auto format = vm_pop(vm);

    if (format.type != VAL_STRING)
    {
        // TODO: proper error reporting
        fprintf(stderr, "First argument of print is the format string.\n");
        exit(EXIT_FAILURE);
    }

    const auto str = format.as_string;

    printf("%s\n", str);
}

// Handler for OP_CALL
bool handle_call(void) {
    const auto vm = get_vm();
    // we ensured that the function name will be in the same chunk
    // so...
    const char* name = vm_read_fn_name(get_vm());
    // get the function object
    const auto fn = get_function(get_vm()->context, name);

    if (strcmp(fn->name, "print") == 0)
    {
        std_out(vm);
        return true;
    }

    if (fn) {
        push_call_frame(vm->call_stack, vm->chunk, vm->ip, vm->registers);
        fn->return_addr.offset = vm->ip;
        vm->chunk = fn->chunk;
        vm->ip = 0;
    }

    return true;
}

// Handler for OP_RETURN
bool handle_return(void) {
    auto vm = get_vm();
    if (SP < 0) {
        fprintf(stderr, "Nothing on stack to return.\n");
        return false;
    }

    BytecodeChunk* previous_chunk = NULL;
    size_t previous_ip = 0;
    Value* previous_registers = NULL;

    if (!pop_call_frame(vm->call_stack, &previous_chunk, &previous_ip, &previous_registers)) {
        fprintf(stderr, "Call stack underflow on OP_RETURN.\n");
        return false;
    }

    // Restore the previous execution context
    vm->chunk = previous_chunk;
    vm->ip = previous_ip;
    memcpy(vm->registers, previous_registers, MAX_REGISTERS);
    // vm_jump_to_chunk(vm, 0);
    // SP = 27;
    return true;
    // Pop return address
    if (SP < 0) {
        fprintf(stderr, "No return address on stack.\n");
        return false;
    }
    Value return_address_val = vm_pop(vm);
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
    // Optionally, vm_push the return value back onto the stack
    vm_push(vm, make_null());
    return true;
}


// Handler for OP_TERNARY
bool handle_ternary(void) {
    auto vm = get_vm();
    if (SP < 2) {
        fprintf(stderr, "Not enough values on stack for TERNARY operation.\n");
        return false;
    }

    Value false_val = vm_pop(vm);
    Value true_val = vm_pop(vm);
    Value condition = vm_pop(vm);

    if (condition.type != VAL_BOOL) {
        fprintf(stderr, "TERNARY operation requires a boolean condition.\n");
        return false;
    }

    Value result = condition.as_boolean ? true_val : false_val;
    vm_push(vm, result);
    return true;
}

// Handler for OP_HALT
bool handle_halt(void) {
    return false;
}

// Handler for OP_NOP
bool handle_nop(void) {
    // No operation; simply continue execution
    get_vm()->ip++;
    return true;
}

// Handler for OP_STORE_VAR
// OP_STORE_VAR <index:uint64_t>
// where index is the index of the variable in the symbol table
inline bool handle_store_var(void) {
    auto vm = get_vm();
    uint16_t variable_index = vm_read_uint16(vm);
    Value val = vm_pop(vm);
    vm->registers[variable_index] = val;
    return true;
}

inline bool handle_load_const_float(void) {
    auto vm = get_vm();
    if (vm->ip + sizeof(double) > vm->size) {
        fprintf(stderr, "Unexpected end of bytecode during LOAD_CONST.\n");
        return false;
    }

    double value;
    memcpy(&value, &vm->bytecode[vm->ip], sizeof(double));
    vm->ip += sizeof(double);
    Value val = make_float(value);
    vm_push(vm, val);
    return true;
}

inline bool handle_load_var(void) {
    auto vm = get_vm();
    uint16_t variable_index = vm_read_uint16(vm);
    vm_push(vm, vm->registers[variable_index]);
    return true;
}

bool handle_enter_scope(void) {
    auto vm = get_vm();
    if (vm->context->symbols) {
        enter_scope(vm->context->symbols);
    } else {
        vm->context->symbols = create_symbol_table();
    }

    return true;
}

bool handle_exit_scope(void) {
    exit_scope(get_vm()->context->symbols);
    return true;
}

inline bool handle_pop(void) {
    vm_pop(get_vm());
    return true;
}

bool handle_push(void) {
    return false;
}

bool handle_save_sp(void) {
    get_vm()->sp_reset = SP;
    return true;
}

bool handle_reset_sp(void) {
    // for (int i = SP; i <= vm->sp_reset; i--) vm_pop(vm);
    SP = get_vm()->sp_reset;
    return true;
}

inline bool handle_inc_reg(void) {
    auto vm = get_vm();
    uint16_t variable_index = vm_read_uint16(vm);
    Value val = vm->registers[variable_index];

    if (val.type == VAL_INT) {
        ((vm->registers) + variable_index)->as_integer++;
    }

    return true;
}

