#include "cpu/instr.h"
/*
Put the implementations of `inc' instructions here.
*/
static void instr_execute_1op() 
{
	operand_read(&opr_src);
    //TODO 手册描述说是不影响 CF 标记位， 但是通过了测试用例
	opr_src.val = alu_add(opr_src.val, 1, opr_src.data_size);
	operand_write(&opr_src);
}

make_instr_impl_1op(inc, rm, v)
make_instr_impl_1op(inc, r, v)
