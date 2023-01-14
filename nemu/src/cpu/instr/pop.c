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

make_instr_func(popa){
    OPERAND r;
    r.data_size = 32;
    r.sreg = SREG_SS;
    r.type = OPR_MEM;
    
    r.addr = cpu.esp;
    operand_read(&r);
    cpu.edi = r.val;
    cpu.esp += 4;

    r.addr = cpu.esp;
    operand_read(&r);
    cpu.esi = r.val;
    cpu.esp += 4;

    r.addr = cpu.esp;
    operand_read(&r);
    cpu.ebp = r.val;
    cpu.esp += 4;

    cpu.esp += 4;

    r.addr = cpu.esp;
    operand_read(&r);
    cpu.ebx = r.val;
    cpu.esp += 4;

    r.addr = cpu.esp;
    operand_read(&r);
    cpu.edx = r.val;
    cpu.esp += 4;

    r.addr = cpu.esp;
    operand_read(&r);
    cpu.ecx = r.val;
    cpu.esp += 4;

    r.addr = cpu.esp;
    operand_read(&r);
    cpu.eax = r.val;
    cpu.esp += 4;

    return 1;
}