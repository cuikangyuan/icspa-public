#include "cpu/instr.h"
/*
Put the implementations of `leave' instructions here.
*/
make_instr_func(leave) {
	int len = 1;
    cpu.esp = cpu.ebp;
    cpu.ebp = paddr_read(cpu.esp, 4);
    cpu.esp += 4;
	return len;
}