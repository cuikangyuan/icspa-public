#include "cpu/instr.h"
/*
Put the implementations of `lea' instructions here.
*/
make_instr_func(lea_rm2r_v) {
	int len = 1;
	OPERAND r, rm;
	r.data_size = data_size;
	rm.data_size = 8;
	len += modrm_r_rm(eip + 1, &r, &rm);
	
	r.val = rm.addr;
	operand_write(&r);

	return len;

	//ref:
	// OPERAND dest, src;
    // dest.data_size = src.data_size = data_size;
    // int len = modrm_r_rm(cpu.eip + 1, &dest, &src);
    // dest.val = src.addr;
    // operand_write(&dest);
    // return 1 + len;
}