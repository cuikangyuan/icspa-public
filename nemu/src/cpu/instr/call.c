#include "cpu/instr.h"
/*
Put the implementations of `call' instructions here.
*/
make_instr_func(call_near)
{
        OPERAND rel;
        rel.type = OPR_IMM;
        rel.data_size = data_size;
        rel.addr = eip + 1;

        operand_read(&rel);

        int offset = sign_ext(rel.val, data_size);

        //push next instr address
        cpu.esp -= data_size / 8;
        paddr_write(cpu.esp, data_size / 8, cpu.eip + 1 + data_size / 8);
        //update eip base on next instr address with rel32
        cpu.eip = cpu.eip + 1 + data_size / 8 + offset;

        return 0;
}
