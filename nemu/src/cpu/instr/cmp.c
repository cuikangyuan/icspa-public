#include "cpu/instr.h"
/*
Put the implementations of `cmp' instructions here.
*/
static void instr_execute_2op() {
    operand_read(&opr_src);
    operand_read(&opr_dest);

    if (opr_dest.data_size > opr_src.data_size)
    {
        opr_src.val = sign_ext(opr_src.val, opr_src.data_size);
    }
    
    //符号扩展 TODO movsx testcase需要使用以下符号扩展
    // if (opr_src.data_size == 8 && (opr_src.val & 0x80) && opr_dest.data_size > 8) {
    //     if (opr_dest.data_size == 16) {
    //         opr_src.val |= 0xff00;
    //     } else {
    //         opr_src.val |= 0xffffff00;
    //     } 
    // }

    alu_sub(opr_src.val, opr_dest.val, data_size);
    //trace_instr("cmp: src %x dest: %x result: %x\n", opr_src.val, opr_dest.val, result);

}

make_instr_impl_2op(cmp, i, rm, v)
make_instr_impl_2op(cmp, i, rm, b)
make_instr_impl_2op(cmp, i, rm, bv)
make_instr_impl_2op(cmp, r, rm, v)
make_instr_impl_2op(cmp, r, rm, b)
make_instr_impl_2op(cmp, rm, r, v)
make_instr_impl_2op(cmp, rm, r, b)
make_instr_impl_2op(cmp, i, a, b)
make_instr_impl_2op(cmp, i, a, v)
