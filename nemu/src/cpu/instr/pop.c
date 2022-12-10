#include "cpu/instr.h"
/*
Put the implementations of `pop' instructions here.
*/
static void instr_execute_1op() {

    OPERAND esp, r;

    esp.data_size = 32;
    esp.type = OPR_REG;
    esp.addr = 0x4;

    operand_read(&esp);

    r.data_size = 32;
    r.type = OPR_MEM;
    r.sreg = SREG_SS;
    r.addr = esp.val;

    operand_read(&r);

    esp.val += 4;

    operand_write(&esp);

    opr_src.val = r.val;
    operand_write(&opr_src);

}

make_instr_impl_1op(pop, r, v)