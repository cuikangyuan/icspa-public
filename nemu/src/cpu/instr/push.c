#include "cpu/instr.h"
/*
Put the implementations of `push' instructions here.
*/
static void instr_execute_1op() {
    
    operand_read(&opr_src);

    OPERAND esp, r;

    esp.data_size = 32;
    esp.type = OPR_REG;
    esp.addr = 0x4;

    operand_read(&esp);
    
    esp.val -= 4;

    r.data_size = 32;
    r.sreg = SREG_SS;
    r.type = OPR_MEM;
    r.addr = esp.val;
    r.val = opr_src.val;

    operand_write(&esp);
    operand_write(&r);
}

make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, i, v)
make_instr_impl_1op(push, i, b)
make_instr_impl_1op(push, rm, v)

make_instr_func(pusha){
    uint32_t temp = cpu.esp;
    OPERAND r;
    r.data_size = 32;
    r.sreg = SREG_SS;
    r.type = OPR_MEM;
    
    cpu.esp -= 4;
    r.val = cpu.eax;
    r.addr = cpu.esp;
    operand_write(&r);

    cpu.esp -= 4;
    r.val = cpu.ecx;
    r.addr = cpu.esp;
    operand_write(&r);

    cpu.esp -= 4;
    r.val = cpu.edx;
    r.addr = cpu.esp;
    operand_write(&r);

    cpu.esp -= 4;
    r.val = cpu.ebx;
    r.addr = cpu.esp;
    operand_write(&r);

    cpu.esp -= 4;
    r.val = temp;
    r.addr = cpu.esp;
    operand_write(&r);

    cpu.esp -= 4;
    r.val = cpu.ebp;
    r.addr = cpu.esp;
    operand_write(&r);

    cpu.esp -= 4;
    r.val = cpu.esi;
    r.addr = cpu.esp;
    operand_write(&r);

    cpu.esp -= 4;
    r.val = cpu.edi;
    r.addr = cpu.esp;
    operand_write(&r);

    return 1;
}
