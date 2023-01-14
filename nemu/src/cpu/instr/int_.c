#include "cpu/intr.h"
#include "cpu/instr.h"

/*
Put the implementations of `int' instructions here.

Special note for `int': please use the instruction name `int_' instead of `int'.
*/
make_instr_func(intr)
{
    //printf("lgdt enter eip=%x\n", eip);
    int len = 0;
    OPERAND imm;
    
    imm.data_size = 8;
    imm.type = OPR_IMM;
    imm.addr = cpu.eip + 1;

    operand_read(&imm);

	print_asm_1("intr", "", len, &imm);

    raise_sw_intr(imm.val);

    return len;
}