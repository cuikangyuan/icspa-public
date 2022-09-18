#include "cpu/instr.h"
/*
Put the implementations of `pop' instructions here.
*/
static void instr_execute_1op() {
    uint32_t esp_val = paddr_read(cpu.esp, data_size / 8);
    opr_src.val = esp_val;
    operand_write(&opr_src);
    cpu.esp += data_size / 8;
}

make_instr_impl_1op(pop, r, v)