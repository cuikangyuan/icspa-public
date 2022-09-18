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
}