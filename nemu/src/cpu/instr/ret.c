#include "cpu/instr.h"
/*
Put the implementations of `ret' instructions here.
*/
make_instr_func(ret_near)
{

    cpu.eip = paddr_read(cpu.esp, data_size / 8);
    cpu.esp += data_size / 8;

    return 0;
}

make_instr_func(ret_near_imm16)
{

    uint16_t imm16 = paddr_read(cpu.eip, 2);
    cpu.eip = paddr_read(cpu.esp, data_size / 8);
    cpu.esp += (data_size / 8 + imm16);

    return 0;
}