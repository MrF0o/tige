//
// Created by fathi on 10/23/2024.
//

#ifndef TIGE_OP_HANDLERS_H
#define TIGE_OP_HANDLERS_H

typedef struct VM VM;

typedef bool (*OpcodeHandler)(void);

bool handle_nop(void);

bool handle_load_const_int(void);

bool handle_load_const_float(void);

bool handle_load_string(void);

bool handle_load_bool(void);

bool handle_load_var(void);

bool handle_store_var(void);

bool handle_add(void);

bool handle_sub(void);

bool handle_mul(void);

bool handle_div(void);

bool handle_and(void);

bool handle_or(void);

bool handle_not(void);

bool handle_equal(void);

bool handle_not_equal(void);

bool handle_less_than(void);

bool handle_greater_than(void);

bool handle_less_equal(void);

bool handle_greater_equal(void);

bool handle_jmp(void);

bool handle_jmp_adr(void);

bool handle_jmp_if_true(void);

bool handle_jmp_if_false(void);

bool handle_call(void);

bool handle_return(void);

bool handle_new_object(VM *vm);

bool handle_get_property(VM *vm);

bool handle_set_property(VM *vm);

bool handle_alloc_heap(VM *vm);

bool handle_free_heap(VM *vm);

bool handle_ternary(void);

bool handle_enter_scope(VM *vm);

bool handle_exit_scope(void);

bool handle_push(void);

bool handle_pop(VM *vm);

bool handle_halt(void);

bool handle_save_sp(void);

bool handle_reset_sp(void);

bool handle_inc_reg(void);

#endif //TIGE_OP_HANDLERS_H
