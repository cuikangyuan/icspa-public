#include "cpu/instr.h"
/*
Put the implementations of `pop' instructions here.
*/
static void instr_execute_1op() {
    // OPERAND esp;
    
    // esp.data_size = data_size;
    // esp.type = OPR_REG;
    // esp.addr = 0x4;
    
    // operand_read(&esp);

    // assert(cpu.esp == esp.val);

    // opr_src.val = esp.val;
    // operand_write(&opr_src);
    // cpu.esp += data_size / 8;

    uint32_t esp_val = paddr_read(cpu.esp, data_size / 8);
    opr_src.val = esp_val;
    operand_write(&opr_src);
    cpu.esp += data_size / 8;



}

make_instr_impl_1op(pop, r, v)