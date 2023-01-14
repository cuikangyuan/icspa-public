#include "cpu/instr.h"
/*
Put the implementations of `lidt' instructions here.
*/
make_instr_func(instr_lidt)
{
    //printf("lgdt enter eip=%x\n", eip);
    int len = 1;
    OPERAND m;
    m.data_size = 16;
    len += modrm_rm(eip + 1, &m);

    //0f 01 18 90 5d c3 55
    operand_read(&m);
    cpu.idtr.limit = m.val;

    m.data_size = 32;
    m.addr += 2;
    operand_read(&m);
    cpu.idtr.base = m.val;
	print_asm_1("lidt", "", len, &m);


    return len;
}