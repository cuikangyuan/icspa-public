#include "cpu/instr.h"
/*
Put the implementations of `sub' instructions here.
*/
static void instr_execute_2op() 
{
	operand_read(&opr_src);
    operand_read(&opr_dest);
    opr_src.val = sign_ext(opr_src.val, opr_src.data_size);//符号扩展
    opr_dest.val = alu_sub(opr_src.val, opr_dest.val, data_size);
	operand_write(&opr_dest);
}


make_instr_impl_2op(sub, i, rm, bv)
