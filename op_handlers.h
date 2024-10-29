//
// Created by fathi on 10/23/2024.
//

#ifndef TIGE_OP_HANDLERS_H
#define TIGE_OP_HANDLERS_H

typedef struct VM VM;

typedef bool (*OpcodeHandler)(VM *vm);

bool handle_nop(VM *vm);

bool handle_load_const_int(VM *vm);

bool handle_load_const_float(VM *vm);

bool handle_load_string(VM *vm);

bool handle_load_bool(VM *vm);

bool handle_load_var(VM *vm);

bool handle_store_var(VM *vm);

bool handle_add(VM *vm);

bool handle_sub(VM *vm);

bool handle_mul(VM *vm);

bool handle_div(VM *vm);

bool handle_and(VM *vm);

bool handle_or(VM *vm);

bool handle_not(VM *vm);

bool handle_equal(VM *vm);

bool handle_not_equal(VM *vm);

bool handle_less_than(VM *vm);

bool handle_greater_than(VM *vm);

bool handle_less_equal(VM *vm);

bool handle_greater_equal(VM *vm);

bool handle_jmp(VM *vm);

bool handle_jmp_adr(VM *vm);

bool handle_jmp_if_true(VM *vm);

bool handle_jmp_if_false(VM *vm);

bool handle_call(VM *vm);

bool handle_return(VM *vm);

bool handle_new_object(VM *vm);

bool handle_get_property(VM *vm);

bool handle_set_property(VM *vm);

bool handle_alloc_heap(VM *vm);

bool handle_free_heap(VM *vm);

bool handle_ternary(VM *vm);

bool handle_enter_scope(VM *vm);

bool handle_exit_scope(VM *vm);

bool handle_push(VM *vm);

bool handle_pop(VM *vm);

bool handle_halt(VM *vm);

bool handle_save_sp(VM *vm);

bool handle_reset_sp(VM *vm);

#endif //TIGE_OP_HANDLERS_H
