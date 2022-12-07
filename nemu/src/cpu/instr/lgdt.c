#include "cpu/instr.h"
/*
Put the implementations of `lgdt' instructions here.
*/
make_instr_func(instr_lgdt)
{
    //printf("lgdt enter eip=%x\n", eip);
    int len = 0;
#ifdef IA32_SEG    
    len = 1;
    OPERAND m;
    m.data_size = 16;
    len += modrm_rm(eip + 1, &m);

    //0f 01 15 4c 00 03 00
    operand_read(&m);
    cpu.gdtr.limit = m.val;

    m.data_size = 32;
    m.addr += 2;
    operand_read(&m);
    cpu.gdtr.base = m.val;
    //printf("base=%x,limit=%x,eip=%x,len=%x\n", cpu.gdtr.base, cpu.gdtr.limit, (eip + 1), len);
	print_asm_1("lgdt", "", len, &m);
#endif

    return len;
}