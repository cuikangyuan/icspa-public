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