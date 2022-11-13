#include "cpu/cpu.h"
#include "memory/memory.h"

// return the linear address from the virtual address and segment selector
uint32_t segment_translate(uint32_t offset, uint8_t sreg)
{
	/* TODO: perform segment translation from virtual address to linear address
	 * by reading the invisible part of the segment register 'sreg'
	 */
	//printf("segment_translate enter , sreg index=%x\n", sreg);
	assert(sreg >= 0 && sreg <= 5);
	return cpu.segReg[sreg].base + offset;
}

// load the invisible part of a segment register
void load_sreg(uint8_t sreg)
{
	/* TODO: load the invisibile part of the segment register 'sreg' by reading the GDT.
	 * The visible part of 'sreg' should be assigned by mov or ljmp already.
	 */

	//printf("load_sreg enter , sreg index=%x\n", sreg);
	assert(sreg >= 0 && sreg <= 5);
	SegDesc* segDesc = (SegDesc* )(hw_mem + cpu.gdtr.base + cpu.segReg[sreg].index * 8);

	uint32_t type = segDesc->type;
	uint32_t base = (segDesc->base_31_24 << 24) + (segDesc->base_23_16 << 16) + segDesc->base_15_0;
	uint32_t limit = (segDesc->limit_19_16 << 16) + segDesc->limit_15_0;
	//printf("load_sreg enter , segDesc: type=%x,base=%x,limit=%x\n", type, base, limit);
	assert(base == 0x0);
	assert(limit == 0xfffff);
	assert(segDesc->granularity == 1);
	assert(segDesc->present == 1);

	SegReg* segReg = cpu.segReg + sreg;
	segReg->type = type;
	segReg->base = base;
	segReg->limit = limit;
	
	
}
