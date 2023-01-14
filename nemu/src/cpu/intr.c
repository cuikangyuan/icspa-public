#include "cpu/intr.h"
#include "cpu/instr.h"
#include "memory/memory.h"

void raise_intr(uint8_t intr_no)
{
#ifdef IA32_INTR
	//printf("raise_intr() enter, intr_no = 0x%x\n", intr_no);
	fflush(stdout);
	// Trigger an exception/interrupt with 'intr_no'
    // 'intr_no' is the index to the IDT
    // Push EFLAGS, CS, and EIP
	OPERAND r;
    r.sreg = SREG_SS;
    r.type = OPR_MEM;
    
    cpu.esp -= 4;
    r.data_size = 32;
    r.addr = cpu.esp;
    r.val = cpu.eflags.val;
    operand_write(&r);

    cpu.esp -= 4;
    r.data_size = 32;
    r.addr = cpu.esp;
    r.val = cpu.cs.val;
    operand_write(&r);
    
    cpu.esp -= 4;
    r.data_size = 32;
    r.addr = cpu.esp;
    r.val = cpu.eip;
    operand_write(&r);
	//???
    // Find the IDT entry using 'intr_no'
    GateDesc* gateDesc = (GateDesc*) (hw_mem + page_translate(segment_translate(cpu.idtr.base + 8 * intr_no, SREG_CS)));

	// Clear IF if it is an interrupt
	//???
	if(gateDesc->type == 0xe){
        cpu.eflags.IF = 0;
    }
    uint32_t addr = (gateDesc->offset_31_16 << 16) + gateDesc->offset_15_0;
    cpu.cs.val = gateDesc->selector;
    //load_sreg(1);
    cpu.eip = addr;
    
	// Set EIP to the entry of the interrupt handler
#endif
}

void raise_sw_intr(uint8_t intr_no)
{
	// return address is the next instruction
	cpu.eip += 2;
	raise_intr(intr_no);
}
