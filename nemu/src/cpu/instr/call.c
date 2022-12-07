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

        //ref:       
        // uint32_t rel = paddr_read(eip + 1, data_size / 8);
        // cpu.esp -= 4;
        // paddr_write(cpu.esp, 4, cpu.eip + 1 + data_size / 8);
        // cpu.eip = (cpu.eip + 1 + data_size / 8 + rel);

        // uint32_t esp_t = cpu.esp - data_size / 8;

        // OPERAND esp;
        // esp.data_size = data_size;
        // esp.type = OPR_REG;
        // esp.addr = 0x4;        
        // operand_read(&esp);

        // esp.val -= data_size / 8;

        // assert(esp.val == esp_t);

        // OPERAND ip;
        // ip.data_size = data_size;
        // ip.type = OPR_MEM;
        // ip.val = cpu.eip + 1 + data_size / 8;
        // ip.addr = esp.val;

        // operand_write(&esp);
        // operand_write(&ip);

        // cpu.eip = cpu.eip + 1 + data_size / 8 + offset;

        return 0;
}

make_instr_func(call_indirect)
{
        OPERAND rm;
        rm.data_size = data_size;
        int len = modrm_rm(eip + 1, &rm);
        operand_read(&rm);

        // cpu.esp -= data_size / 8;
        // paddr_write(cpu.esp, data_size / 8, cpu.eip + 1 + len);
        // cpu.eip = rm.val;

        uint32_t esp_t = cpu.esp - data_size / 8;

        OPERAND esp;
        esp.data_size = data_size;
        esp.type = OPR_REG;
        esp.addr = 0x4;        
        operand_read(&esp);

        esp.val -= data_size / 8;

        assert(esp.val == esp_t);

        OPERAND ip;
        ip.data_size = data_size;
        ip.type = OPR_MEM;
        ip.val = cpu.eip + 1 + len;
        ip.addr = esp.val;

        operand_write(&esp);
        operand_write(&ip);        

        cpu.eip = rm.val;



        return 0;
}
