#include "cpu/instr.h"
/*
Put the implementations of `leave' instructions here.
*/
make_instr_func(leave) {
	int len = 1;

    cpu.esp = cpu.ebp;
    cpu.ebp = paddr_read(cpu.esp, 4);
    cpu.esp += 4;

    // uint32_t esp_t = cpu.ebp;
    // uint32_t ebp_t = paddr_read(esp_t, 4);;
    // esp_t += 4;

    // OPERAND r, esp, ebp;
    
    // esp.data_size = data_size;
    // esp.type = OPR_REG;
    // esp.addr = 0x4;
    
    // ebp.data_size = data_size;
    // ebp.type = OPR_REG;
    // ebp.addr = 0x5;
    
    // operand_read(&ebp);
    
    // esp.val = ebp.val;
    
    // r.data_size = data_size;
    // r.type = OPR_MEM;
    // r.sreg = SREG_SS;
    // r.addr = esp.val;


    // operand_read(&r);
    
    // esp.val += data_size / 8;
    
    // ebp.val = r.val;

    // operand_write(&esp);
    // operand_write(&ebp);

    // assert(ebp_t == ebp.val);
    // assert(esp_t == esp.val);

	return len;
}