#include "cpu/instr.h"
/*
Put the implementations of `ret' instructions here.
*/
make_instr_func(ret_near)
{

    cpu.eip = paddr_read(cpu.esp, data_size / 8);
    cpu.esp += data_size / 8;
    
    // uint32_t eip_t = paddr_read(cpu.esp, data_size / 8);
    // uint32_t esp_t = cpu.esp + data_size / 8;


    // OPERAND esp,ip;
    
    // esp.data_size = data_size;
    // esp.type = OPR_REG;
    // esp.addr = 0x4;
    
    // operand_read(&esp);
    
    // ip.data_size = data_size;
    // ip.type = OPR_MEM;
    // ip.sreg = SREG_SS;
    // ip.addr = esp.val;
    
    // operand_read(&ip);
    
    // esp.val += data_size / 8;
    
    // cpu.eip = ip.val;

    // assert(cpu.eip == eip_t);
    // assert(esp.val == esp_t);
    
    // operand_write(&esp);

    return 0;
}

make_instr_func(ret_near_imm16)
{

    uint16_t imm16 = paddr_read(cpu.eip, 2);
    cpu.eip = paddr_read(cpu.esp, data_size / 8);
    cpu.esp += (data_size / 8 + imm16);

    // uint16_t imm16_t = paddr_read(cpu.eip, 2);
    // uint32_t eip_t = paddr_read(cpu.esp, data_size / 8);
    // uint32_t esp_t = cpu.esp + data_size / 8 + imm16_t;

    // OPERAND esp,ip,imm;
    
    // imm.data_size = 16;
    // imm.type = OPR_IMM;
    // imm.addr = cpu.eip;
    
    // operand_read(&imm);

    // assert(imm.val == imm16_t);

    // esp.data_size = data_size;
    // esp.type = OPR_REG;
    // esp.addr = 0x4;
    
    // operand_read(&esp);

    // assert(esp.val == cpu.esp);
    
    // ip.data_size = data_size;
    // ip.type = OPR_MEM;
    // ip.sreg = SREG_SS;
    // ip.addr = esp.val;
    
    // operand_read(&ip);

    // assert(ip.val == eip_t);

    // cpu.eip = ip.val;
    
    // esp.val += data_size / 8;
    // esp.val += imm.val;
    
    // assert(esp.val == esp_t);

    // operand_write(&esp);


    return 0;
}