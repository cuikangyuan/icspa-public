#include "cpu/cpu.h"

inline void clear_CF() {
	if (cpu.eflags.CF != 0) 
	{
		cpu.eflags.CF = 0;
	}
}
inline void clear_OF() {
	if (cpu.eflags.OF != 0) 
	{
		cpu.eflags.OF = 0;
	}
}

void set_CF_sub(uint32_t result, uint32_t src, uint32_t dest, size_t data_size)
{
	//参考i386手册 sec 2.3.4.1
	result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
	//src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
	dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
	cpu.eflags.CF = result > dest;
}

void set_CF_sbb(uint32_t result, uint32_t src, uint32_t dest, size_t data_size)
{
	result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
	dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
	if (cpu.eflags.CF == 0)
	{
		cpu.eflags.CF = result > dest;
	} else if (cpu.eflags.CF == 1)
	{
		cpu.eflags.CF = result >= dest;
	} else {
		//nothing
	}
}

/*
Carry Flag
Set on high-order bit carry or borrow; cleared otherwise
*/
void set_CF_adc(uint32_t result, uint32_t src, uint32_t dest, size_t data_size)
{
	result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
	src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
	dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
	if (cpu.eflags.CF == 0)
	{
		cpu.eflags.CF = result < src;
	} else if (cpu.eflags.CF == 1)
	{
		cpu.eflags.CF = result <= src;
	} else {
		//nothing
	}
	
}

/*
Carry Flag
Set on high-order bit carry or borrow; cleared otherwise
*/
void set_CF_add(uint32_t result, uint32_t src, size_t data_size)
{
	//参考i386手册 sec 2.3.4.1
	result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
	src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
	cpu.eflags.CF = result < src;
}

/*
Zero Flag ── Set if result is zero; cleared otherwise.
*/
void set_ZF(uint32_t result, size_t data_size)
{
	result = result & (0xFFFFFFFF >> (32 - data_size));
	cpu.eflags.ZF = (result == 0);
}

/*
Sign Flag ── Set equal to high-order bit of result (0 is
positive, 1 if negative).
*/
void set_SF(uint32_t result, size_t data_size)
{
	result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
	cpu.eflags.SF = sign(result);
}

/*
Parity Flag ── Set if low-order eight bits of result contain
an even number of 1 bits; cleared otherwise.
*/
void set_PF(uint32_t result)
{
	int even = 0;
	switch (result & 0xf)
	{
	case 0x0:
	case 0x3:
	case 0x5:
	case 0x6:
	case 0x9:
	case 0xa:
	case 0xc:
	case 0xf:
		even = 0; //even num
		break;
	default:
		even = 1; //odd num
		break;
	}
	switch ((result >> 4) & 0xf)
	{
	case 0x0:
	case 0x3:
	case 0x5:
	case 0x6:
	case 0x9:
	case 0xa:
	case 0xc:
	case 0xf:
		if (even == 0)
		{
			cpu.eflags.PF = 1;
		} else {
			cpu.eflags.PF = 0;
		}
		break;
	default:
		if (even == 1)
		{
			cpu.eflags.PF = 1;
		} else {
			cpu.eflags.PF = 0;
		}
		break;
	}


}

/*
Overflow Flag ── Set if result is too large a positive number
or too small a negative number (excluding sign-bit) to fit in
destination operand; cleared otherwise.
*/

void set_OF(uint32_t result, uint32_t src, uint32_t dest, size_t data_size)
{
	switch (data_size)
	{
	case 8:
		result = sign_ext(result & 0xFF, 8);
		src = sign_ext(src & 0xFF, 8);
		dest = sign_ext(dest & 0xFF, 8);
		break;
	case 16:
		result = sign_ext(result & 0xFFFF, 16);
		src = sign_ext(src & 0xFFFF, 16);
		dest = sign_ext(dest & 0xFFFF, 16);
		break;
	default:
		//nothing
		break;
	}
	if (sign(src) == sign(dest))
	{
		if (sign(src) != sign(result))
		{
			cpu.eflags.OF = 1;
		} else {
			cpu.eflags.OF = 0;
		}
		
	} else {
		cpu.eflags.OF = 0;
	}
	
}

void set_OF_sub(uint32_t result, uint32_t src, uint32_t dest, size_t data_size)
{
	switch (data_size)
	{
	case 8:
		result = sign_ext(result & 0xFF, 8);
		src = sign_ext(src & 0xFF, 8);
		dest = sign_ext(dest & 0xFF, 8);
		break;
	case 16:
		result = sign_ext(result & 0xFFFF, 16);
		src = sign_ext(src & 0xFFFF, 16);
		dest = sign_ext(dest & 0xFFFF, 16);
		break;
	default:
		//nothing
		break;
	}
	if (sign(src) != sign(dest))
	{
		if (sign(dest) != sign(result))
		{
			cpu.eflags.OF = 1;
		} else {
			cpu.eflags.OF = 0;
		}
		
	} else {
		cpu.eflags.OF = 0;
	}
	
}

uint32_t alu_add(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_add(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest + src;

	set_CF_add(res, src, data_size);
	set_PF(res);
	// set_AF(); //不模拟
	set_ZF(res, data_size);
	set_SF(res, data_size);
	set_OF(res, src, dest, data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_adc(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_adc(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest + src + cpu.eflags.CF;

	set_CF_adc(res, src, dest, data_size);
	set_PF(res);
	// set_AF(); //不模拟
	set_ZF(res, data_size);
	set_SF(res, data_size);
	set_OF(res, src, dest, data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_sub(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sub(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest - src;
	//printf("alu_sub !! dest=%#010x, src=%#010x, res=%#010x, data_size=%#010x\n", dest, src, res, data_size);
	set_CF_sub(res, src, dest, data_size);
	set_PF(res);
	// set_AF(); //不模拟
	set_ZF(res, data_size);
	set_SF(res, data_size);
	set_OF_sub(res, src, dest, data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_sbb(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sbb(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest - src - cpu.eflags.CF;

	set_CF_sbb(res, src, dest, data_size);
	set_PF(res);
	// set_AF(); //不模拟
	set_ZF(res, data_size);
	set_SF(res, data_size);
	set_OF_sub(res, src, dest, data_size);
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint64_t alu_mul(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mul(src, dest, data_size);
#else
	uint64_t src64 = src;
	uint64_t dest64 = dest;
	uint64_t res = src64 * dest64;
	uint64_t high_orders = res & (0xFFFFFFFFFFFFFFFF << data_size);
	//printf("src64=%#018llx, src=%#010x\n", src64, src);
	if (high_orders != 0)
	{
		cpu.eflags.CF = 1;
		cpu.eflags.OF = 1;
		//printf("CF set !! dest=%#010x, src=%#010x, res=%#018llx, data_size=%#010x, mask=%#010x, high_orders=%#018llx\n", dest, src, res, data_size, mask, high_orders);
	} else {
		cpu.eflags.CF = 0;
		cpu.eflags.OF = 0;
		//printf("CF clear !! dest=%#010x, src=%#010x, res=%#018llx, data_size=%#010x, mask=%#010x, high_orders=%#018llx\n", dest, src, res, data_size, mask, high_orders);
	}
	
	return res;
#endif
}

int64_t alu_imul(int32_t src, int32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imul(src, dest, data_size);
#else
	//uint32_t mask = ((uint32_t) 0xFFFFFFFF >> (32 - data_size));
	int64_t src64 = src;
	int64_t dest64 = dest;
	int64_t res = dest64 * src64;
	uint64_t high_orders = res & (0xFFFFFFFFFFFFFFFF << data_size);
	if (high_orders != 0)
	{
		cpu.eflags.CF = 1;
		cpu.eflags.OF = 1;
		//printf("CF set !! dest=%#010x, src=%#010x, res=%#018llx, data_size=%#010x, mask=%#010x, high_orders=%#018llx\n", dest, src, res, data_size, mask, high_orders);
	} else {
		cpu.eflags.CF = 0;
		cpu.eflags.OF = 0;
		//printf("CF clear !! dest=%#010x, src=%#010x, res=%#018llx, data_size=%#010x, mask=%#010x, high_orders=%#018llx\n", dest, src, res, data_size, mask, high_orders);
	}
	
	return res;
#endif
}

// need to implement alu_mod before testing
uint32_t alu_div(uint64_t src, uint64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_div(src, dest, data_size);
#else
	uint32_t mask = ((uint32_t) 0xFFFFFFFF >> (32 - data_size));
	uint32_t res = dest / src;
	return res & mask;
#endif
}

// need to implement alu_imod before testing
int32_t alu_idiv(int64_t src, int64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_idiv(src, dest, data_size);
#else
	uint32_t mask = ((uint32_t) 0xFFFFFFFF >> (32 - data_size));
	int32_t res = dest / src;
	return res & mask;
#endif
}

uint32_t alu_mod(uint64_t src, uint64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mod(src, dest);
#else
	uint32_t res = dest % src; 
	return res;
#endif
}

int32_t alu_imod(int64_t src, int64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imod(src, dest);
#else
	int32_t res = dest % src;
	return res;
#endif
}

uint32_t alu_and(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_and(src, dest, data_size);
#else
	uint32_t res = src & dest;

	clear_CF();
	set_PF(res);
	// set_AF(); //不模拟
	set_ZF(res, data_size);
	set_SF(res, data_size);
	clear_OF();
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_xor(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_xor(src, dest, data_size);
#else
	uint32_t res = src ^ dest;

	clear_CF();
	set_PF(res);
	// set_AF(); //不模拟
	set_ZF(res, data_size);
	set_SF(res, data_size);
	clear_OF();
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_or(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_or(src, dest, data_size);
#else
	uint32_t res = src | dest;

	clear_CF();
	set_PF(res);
	// set_AF(); //不模拟
	set_ZF(res, data_size);
	set_SF(res, data_size);
	clear_OF();
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

//逻辑左移
uint32_t alu_shl(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shl(src, dest, data_size);
#else
	uint32_t shift_count = src & (0xFFFFFFFF >> (32 - data_size));
	uint32_t res = dest << shift_count;
	cpu.eflags.CF = (dest >> (data_size - shift_count)) & 0x1;
	set_PF(res);
	// set_AF(); //不模拟
	set_ZF(res, data_size);
	set_SF(res, data_size);
	//clear_OF();
	return res & (0xFFFFFFFF >> (32 - data_size));
#endif
}

//逻辑右移
uint32_t alu_shr(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shr(src, dest, data_size);
#else
	uint32_t shift_count = src & (0xFFFFFFFF >> (32 - data_size));
	uint32_t res = dest & (0xFFFFFFFF >> (32 - data_size));
	if (shift_count != 0)
	{
		cpu.eflags.CF = (res >> (shift_count - 1)) & 0x1;
		res = res >> shift_count;
	}

	set_PF(res);
	// set_AF(); //不模拟
	set_ZF(res, data_size);
	set_SF(res, data_size);
	//clear_OF();
	return res;
#endif
}

//算数右移
uint32_t alu_sar(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sar(src, dest, data_size);
#else

	uint32_t mask = ((uint32_t) 0xFFFFFFFF >> (32 - data_size));
	src = src & mask;
	dest = dest & mask;
	uint32_t res = dest;
	//printf("1.dest=%#010x, src=%#010x, res=%#010x, data_size=%#010x, mask=%#010x\n", dest, src, res, data_size, mask);
	if (src != 0)
	{
		dest = sign_ext(dest, data_size);
		//printf("2.dest=%#010x, src=%#010x, res=%#010x, data_size=%#010x, mask=%#010x\n", dest, src, res, data_size, mask);
		cpu.eflags.CF = (dest >> (src - 1)) & 0x1;
		res = (int32_t)dest >> src;
	}

	set_PF(res);
	// set_AF(); //不模拟
	set_ZF(res, data_size);
	set_SF(res, data_size);
	//clear_OF();
	res = res & mask;
	//printf("3.dest=%#010x, src=%#010x, res=%#010x, data_size=%#010x, mask=%#010x\n", dest, src, res, data_size, mask);
	return res;
#endif
}

//算数左移
uint32_t alu_sal(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sal(src, dest, data_size);
#else
	return alu_shl(src, dest, data_size);
#endif
}
