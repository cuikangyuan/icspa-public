#include "cpu/instr.h"
/*
Put the implementations of `cmp' instructions here.
*/
static void instr_execute_2op() {
    operand_read(&opr_src);
    operand_read(&opr_dest);

    //符号扩展
    if (opr_src.data_size == 8 && (opr_src.val & 0x80) && opr_dest.data_size > 8) {
        if (opr_dest.data_size == 16) {
            opr_src.val |= 0xff00;
        } else {
            opr_src.val |= 0xffffff00;
        } 
    }
    //opr_src.val = sign_ext(opr_src.val, opr_src.data_size);
    alu_sub(opr_src.val, opr_dest.val, data_size);
}

make_instr_impl_2op(cmp, i, rm, bv)
