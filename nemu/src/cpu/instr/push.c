#include "cpu/instr.h"
/*
Put the implementations of `push' instructions here.
*/
static void instr_execute_1op() {
    operand_read(&opr_src);
    cpu.esp -= data_size / 8;
    
    //paddr_write(cpu.esp, data_size / 8, opr_src.val);

    OPERAND r;
    r.addr = cpu.esp;
    r.data_size = 32;
    r.sreg = SREG_SS;
    r.type = OPR_MEM;
    r.val = opr_src.val;
    operand_write(&r);
}

make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, i, v)
make_instr_impl_1op(push, i, b)
make_instr_impl_1op(push, rm, v)
