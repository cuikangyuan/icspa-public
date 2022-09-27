#include "cpu/instr.h"
/*
Put the implementations of `sbb' instructions here.
*/
static void instr_execute_2op() 
{
	operand_read(&opr_src);
    operand_read(&opr_dest);
    //符号扩展
    opr_src.val = sign_ext(opr_src.val, opr_src.data_size);
    // if (opr_src.data_size == 8 && (opr_src.val & 0x80)) {
    //     opr_src.val |= 0xffffff00;
    // }
    opr_dest.val = alu_sbb(opr_src.val, opr_dest.val, data_size);
	operand_write(&opr_dest);
}


make_instr_impl_2op(sbb, r, rm, b)