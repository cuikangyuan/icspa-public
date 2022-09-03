#include "nemu.h"
#include "cpu/fpu.h"

FPU fpu;
// special values
FLOAT p_zero, n_zero, p_inf, n_inf, p_nan, n_nan;


void traceFpuResult(FLOAT res, FLOAT std) {

	printf("res => fval: %f, val: %x, sign: %x, exp: %x, fraction: %x\n", res.fval, res.val, res.sign, res.exponent, res.fraction);
	printf("std => fval: %f, val: %x, sign: %x, exp: %x, fraction: %x\n", std.fval, std.val, std.sign, std.exponent, std.fraction);
}

inline FLOAT internal_normalize_wrapper(uint32_t sign,
										int32_t exp,
										uint64_t sig_grs)
{
	FLOAT f;
	uint32_t res = internal_normalize(sign, exp, sig_grs);
	f.val = res;
	return f;
}

// the last three bits of the significand are reserved for the GRS bits
inline uint32_t internal_normalize(uint32_t sign, //结果的符号
												  //中间结果阶数(含偏置常数，可能为负)
								   int32_t exp,
								   uint64_t sig_grs //中间结果尾数，26位小数, 高38位表示整数部分
)
{

	// normalization
	bool overflow = false; // true if the result is INFINITY or 0 during normalize

	if ((sig_grs >> (23 + 3)) > 1 || exp < 0)
	{
		// normalize toward right
		while ((((sig_grs >> (23 + 3)) > 1) && exp < 0xff) // condition 1
			   ||										   // or
			   (sig_grs > 0x04 && exp < 0)				   // condition 2 ???
		)
		{

			//右规操作
			/* shift right, pay attention to sticky bit*/
			uint32_t sticky = 0;
			sticky = sticky | (sig_grs & 0x1);
			sig_grs = sig_grs >> 1;
			sig_grs |= sticky;
			exp++;
		}

		if (exp >= 0xff)
		{
			//阶码上溢
			/*assign the number to infinity */
			exp = 0xff;
			sig_grs = 0;
			overflow = true;
		}
		if (exp == 0)
		{
			// we have a denormal here, the exponent is 0, but means 2^-126,
			// as a result, the significand should shift right once more
			/* shift right, pay attention to sticky bit*/
			uint32_t sticky = 0;
			sticky = sticky | (sig_grs & 0x1);
			sig_grs = sig_grs >> 1;
			sig_grs |= sticky;
		}
		if (exp < 0)
		{
			/* assign the number to zero */
			exp = 0;
			sig_grs = 0;
			overflow = true;
		}
	}
	else if (((sig_grs >> (23 + 3)) == 0) && exp > 0)
	{
		// normalize toward left
		while (((sig_grs >> (23 + 3)) == 0) && exp > 0)
		{
			/*shift left */
			sig_grs = sig_grs << 1;
			exp--;
		}
		if (exp == 0)
		{
			// denormal
			/* shift right, pay attention to sticky bit*/
			uint32_t sticky = 0;
			sticky = sticky | (sig_grs & 0x1);
			sig_grs = sig_grs >> 1;
			sig_grs |= sticky;
		}
	}
	else if (exp == 0 && sig_grs >> (23 + 3) == 1)
	{
		// two denormals result in a normal
		exp++;
	}

	if (!overflow)
	{
		/* round up and remove the GRS bits */
		// 1.舍入：就近舍入到偶数
		/**
		 * 就近舍入（round to nearest）这是标准列出的默认舍入方式，
		 * 其含义相当于我们日常所说的“四舍五入”。
		 * 例如，对于32位单精度浮点数来说，若超出可保存的23位的多余位大于等于100…01，
		 * 则多余位的值超过了最低可表示位值的一半，这种情况下，舍入的方法是在尾数的最低有效位上加1；
		 * 若多余位小于等于011…11，则直接舍去；若多余位为100…00，此时再判断尾数的最低有效位的值，若为0则直接舍去，若为1则再加1。
		 *
		 */
		// 2.判读是否需要再次右规操作，并判断阶码上溢
		// 3.对于规格化浮点数，丢弃最高位隐藏位

		uint8_t grs = sig_grs & 0x7;
		sig_grs >>= 3;

		bool sig_grs_added = false;

		if (grs == 0x4)
		{
			if (sig_grs & 0x1)
			{
				sig_grs += 0x1;
				sig_grs_added = true;
			}
		}
		else if (grs > 0x4)
		{
			sig_grs += 0x1;
			sig_grs_added = true;
		}

		if (sig_grs_added)
		{
			if ((sig_grs >> 23) > 1)
			{
				sig_grs = sig_grs >> 1;
				exp++;

				if (exp >= 0xff)
				{
					exp = 0xff;
					overflow = true;
					sig_grs = 0;
				}
			}
		}

		if (exp != 0)
		{
			sig_grs &= 0x7fffff;
		}
	}

	FLOAT f;
	f.sign = sign;
	f.exponent = (uint32_t)(exp & 0xff);
	f.fraction = sig_grs; // here only the lowest 23 bits are kept

	return f.val;
}

CORNER_CASE_RULE corner_add[] = {
	{P_ZERO_F, P_ZERO_F, P_ZERO_F},
	{N_ZERO_F, P_ZERO_F, P_ZERO_F},
	{P_ZERO_F, N_ZERO_F, P_ZERO_F},
	{N_ZERO_F, N_ZERO_F, N_ZERO_F},
	{P_INF_F, N_INF_F, N_NAN_F},
	{N_INF_F, P_INF_F, N_NAN_F},
	{P_INF_F, P_NAN_F, P_NAN_F},
	{P_INF_F, N_NAN_F, N_NAN_F},
	{N_INF_F, P_NAN_F, P_NAN_F},
	{N_INF_F, N_NAN_F, N_NAN_F},
	{N_NAN_F, P_NAN_F, P_NAN_F},
};

// a + b
/*
1.处理边界情况 (NaN, 0, INF)
2.提取符号，阶码，尾数
3.整数运算得到中间结果
	3.1 对阶，小阶向大阶看齐
	3.2 尾数相加
4.舍入并规格化后返回
  阶码上溢出 ，参照表格2.2，可能赋值成正无穷等
*/
uint32_t internal_float_add(uint32_t b, uint32_t a)
{

	// corner cases
	int i = 0;
	for (; i < sizeof(corner_add) / sizeof(CORNER_CASE_RULE); i++)
	{
		if (a == corner_add[i].a && b == corner_add[i].b)
			return corner_add[i].res;
	}
	if (a == P_ZERO_F || a == N_ZERO_F)
	{
		return b;
	}
	if (b == P_ZERO_F || b == N_ZERO_F)
	{
		return a;
	}

	FLOAT f, fa, fb;
	fa.val = a;
	fb.val = b;
	// infity, NaN
	if (fa.exponent == 0xff)
	{
		return a;
	}
	if (fb.exponent == 0xff)
	{
		return b;
	}

	// fa中保留阶小的数，fb中保留阶大的数
	//小阶向大阶看齐
	if (fa.exponent > fb.exponent)
	{
		fa.val = b;
		fb.val = a;
	}

	uint32_t sig_a, sig_b, sig_res;
	//提取符号 阶码 尾数部分 规格化数的尾数真值需要在原来尾数基础上加上隐含1
	sig_a = fa.fraction;
	if (fa.exponent != 0)
		sig_a |= 0x800000; // the implied 1
	sig_b = fb.fraction;
	if (fb.exponent != 0)
		sig_b |= 0x800000; // the implied 1

	// alignment shift for fa
	uint32_t shift = 0;
	//将移位的位数计算出来 将阶较小的数的尾数右移，计算shift注意非规格化数的情形
	if (fa.exponent == 0)
	{
		if (fb.exponent == 0)
		{
			shift = fb.exponent - fa.exponent;
		}
		else
		{
			shift = fb.exponent - 127 + 126;
		}
	}
	else
	{
		shift = fb.exponent - fa.exponent;
	}

	// uint32_t shift1 = (fb.exponent == 0 ? fb.exponent + 1 : fb.exponent) - (fa.exponent == 0 ? fa.exponent + 1 : fa.exponent);
	// printf("shift: %x, shift1: %x\n",
	// 		  shift, shift1);

	assert(shift >= 0);

	//右移时的保护位  G R S
	// S位 有1的话 会一直保留着
	// 1位隐藏位 23位尾数(fraction)  3位保护位
	//预留低位用于 guard, round, sticky
	//尾数左移留出GRS bits
	sig_a = (sig_a << 3);
	sig_b = (sig_b << 3);

	//尾数右移对阶
	uint32_t sticky = 0;
	while (shift > 0)
	{
		sticky = sticky | (sig_a & 0x1);
		sig_a = sig_a >> 1;
		sig_a |= sticky;
		shift--;
	}

	//无符号数*-1 再赋值给无符号数 等同于取补码
	// significand add
	if (fa.sign)
	{
		sig_a *= -1; //取补码
	}
	if (fb.sign)
	{
		sig_b *= -1;
	}

	sig_res = sig_a + sig_b; //中间结果尾数

	if (sign(sig_res))
	{
		f.sign = 1;
		sig_res *= -1;
	}
	else
	{
		f.sign = 0;
	}

	uint32_t exp_res = fb.exponent;
	//uint32_t res = internal_normalize(f.sign, exp_res, sig_res);
	FLOAT res = internal_normalize_wrapper(f.sign, exp_res, sig_res);

	//FLOAT test_result;
	//test_result.fval = fa.fval + fb.fval;

	//traceFpuResult(res, test_result);

	return res.val;
}


CORNER_CASE_RULE corner_sub[] = {
	{P_ZERO_F, P_ZERO_F, P_ZERO_F},
	{N_ZERO_F, P_ZERO_F, N_ZERO_F},
	{P_ZERO_F, N_ZERO_F, P_ZERO_F},
	{N_ZERO_F, N_ZERO_F, P_ZERO_F},
	{P_NAN_F, P_NAN_F, P_NAN_F},
	{N_NAN_F, P_NAN_F, P_NAN_F},
	{P_NAN_F, N_NAN_F, P_NAN_F},
	{N_NAN_F, N_NAN_F, N_NAN_F},
};

// a - b
uint32_t internal_float_sub(uint32_t b, uint32_t a)
{
	// change the sign of b
	int i = 0;
	for (; i < sizeof(corner_sub) / sizeof(CORNER_CASE_RULE); i++)
	{
		if (a == corner_sub[i].a && b == corner_sub[i].b)
			return corner_sub[i].res;
	}
	if (a == P_NAN_F || a == N_NAN_F)
		return a;
	if (b == P_NAN_F || b == N_NAN_F)
		return b;
	FLOAT fb;
	fb.val = b;
	fb.sign = ~fb.sign;
	return internal_float_add(fb.val, a);
}

CORNER_CASE_RULE corner_mul[] = {
	{P_ZERO_F, P_INF_F, N_NAN_F},
	{P_ZERO_F, N_INF_F, N_NAN_F},
	{N_ZERO_F, P_INF_F, N_NAN_F},
	{N_ZERO_F, N_INF_F, N_NAN_F},
	{P_INF_F, P_ZERO_F, N_NAN_F},
	{P_INF_F, N_ZERO_F, N_NAN_F},
	{N_INF_F, P_ZERO_F, N_NAN_F},
	{N_INF_F, N_ZERO_F, N_NAN_F},
};

// a * b
uint32_t internal_float_mul(uint32_t b, uint32_t a)
{
	int i = 0;
	for (; i < sizeof(corner_mul) / sizeof(CORNER_CASE_RULE); i++)
	{
		if (a == corner_mul[i].a && b == corner_mul[i].b)
			return corner_mul[i].res;
	}

	if (a == P_NAN_F || a == N_NAN_F || b == P_NAN_F || b == N_NAN_F)
		return a == P_NAN_F || b == P_NAN_F ? P_NAN_F : N_NAN_F;

	FLOAT fa, fb, f;
	fa.val = a;
	fb.val = b;
	f.sign = fa.sign ^ fb.sign;

	if (a == P_ZERO_F || a == N_ZERO_F)
		return fa.sign ^ fb.sign ? N_ZERO_F : P_ZERO_F;
	if (b == P_ZERO_F || b == N_ZERO_F)
		return fa.sign ^ fb.sign ? N_ZERO_F : P_ZERO_F;
	if (a == P_INF_F || a == N_INF_F)
		return fa.sign ^ fb.sign ? N_INF_F : P_INF_F;
	if (b == P_INF_F || b == N_INF_F)
		return fa.sign ^ fb.sign ? N_INF_F : P_INF_F;

	uint64_t sig_a, sig_b, sig_res;
	sig_a = fa.fraction;
	if (fa.exponent != 0)
		sig_a |= 0x800000; // the implied 1
	sig_b = fb.fraction;
	if (fb.exponent != 0)
		sig_b |= 0x800000; // the implied 1

	if (fa.exponent == 0)
		fa.exponent++;
	if (fb.exponent == 0)
		fb.exponent++;

	sig_res = sig_a * sig_b; // 24b * 24b
	uint32_t exp_res = 0;

	/*exp_res = ? leave space for GRS bits. */
	exp_res = fa.exponent + fb.exponent - 127 - (46 - 26);
	return internal_normalize(f.sign, exp_res, sig_res);
}

CORNER_CASE_RULE corner_div[] = {
	{P_ZERO_F, P_ZERO_F, N_NAN_F},
	{N_ZERO_F, P_ZERO_F, N_NAN_F},
	{P_ZERO_F, N_ZERO_F, N_NAN_F},
	{N_ZERO_F, N_ZERO_F, N_NAN_F},
	{P_INF_F, P_ZERO_F, P_INF_F},
	{N_INF_F, P_ZERO_F, N_INF_F},
	{P_INF_F, N_ZERO_F, N_INF_F},
	{N_INF_F, N_ZERO_F, P_INF_F},
	{P_INF_F, P_INF_F, N_NAN_F},
	{N_INF_F, P_INF_F, N_NAN_F},
	{P_INF_F, N_INF_F, N_NAN_F},
	{N_INF_F, N_INF_F, N_NAN_F},
};
// a / b
uint32_t internal_float_div(uint32_t b, uint32_t a)
{

	int i = 0;
	for (; i < sizeof(corner_div) / sizeof(CORNER_CASE_RULE); i++)
	{
		if (a == corner_div[i].a && b == corner_div[i].b)
			return corner_div[i].res;
	}

	FLOAT f, fa, fb;
	fa.val = a;
	fb.val = b;

	if (a == P_NAN_F || a == N_NAN_F || b == P_NAN_F || b == N_NAN_F)
		return a == P_NAN_F || b == P_NAN_F ? P_NAN_F : N_NAN_F;
	if (a == P_INF_F || a == N_INF_F)
	{
		return fa.sign ^ fb.sign ? N_INF_F : P_INF_F;
	}
	if (b == P_ZERO_F || b == N_ZERO_F)
	{
		return fa.sign ^ fb.sign ? N_INF_F : P_INF_F;
	}
	if (a == P_ZERO_F || a == N_ZERO_F)
	{
		fa.sign = fa.sign ^ fb.sign;
		return fa.val;
	}
	if (b == P_INF_F || b == N_INF_F)
	{
		return fa.sign ^ fb.sign ? N_ZERO_F : P_ZERO_F;
	}

	f.sign = fa.sign ^ fb.sign;

	uint64_t sig_a, sig_b, sig_res;
	sig_a = fa.fraction;
	if (fa.exponent != 0)
		sig_a |= 0x800000; // the implied 1
	sig_b = fb.fraction;
	if (fb.exponent != 0)
		sig_b |= 0x800000; // the implied 1

	// efforts to maintain the precision of the result
	int shift = 0;
	while (sig_a >> 63 == 0)
	{
		sig_a <<= 1;
		shift++;
	}
	while ((sig_b & 0x1) == 0)
	{
		sig_b >>= 1;
		shift++;
	}

	sig_res = sig_a / sig_b;

	if (fa.exponent == 0)
		fa.exponent++;
	if (fb.exponent == 0)
		fb.exponent++;
	uint32_t exp_res = fa.exponent - fb.exponent + 127 - (shift - 23 - 3);
	return internal_normalize(f.sign, exp_res, sig_res);
}

void fpu_load(uint32_t val)
{
	fpu.status.top = fpu.status.top == 0 ? 7 : fpu.status.top - 1;
	fpu.regStack[fpu.status.top].val = val;
}

uint32_t fpu_store()
{
	uint32_t val = fpu.regStack[fpu.status.top].val;
	fpu.status.top = (fpu.status.top + 1) % 8;
	return val;
}

uint32_t fpu_peek()
{
	uint32_t val = fpu.regStack[fpu.status.top].val;
	return val;
}

void fpu_add(uint32_t val)
{
	/*
	float *a = (float*)&fpu.regStack[fpu.status.top].val;
	float *b = (float*)&val;
	float c = *a + *b;
	uint32_t *d = (uint32_t *)&c;
	fpu.regStack[fpu.status.top].val = *d;
	*/
	fpu.regStack[fpu.status.top].val = internal_float_add(val, fpu.regStack[fpu.status.top].val);
}

void fpu_add_idx(uint32_t idx, uint32_t store_idx)
{
	/*
	float *a = (float*)&fpu.regStack[fpu.status.top].val;
	float *b = (float*)&fpu.regStack[(fpu.status.top + idx) % 8].val;
	float c = *a + *b;
	uint32_t *d = (uint32_t *)&c;
	fpu.regStack[(fpu.status.top + store_idx) % 8].val = *d;
	*/
	uint32_t a = fpu.regStack[fpu.status.top].val;
	uint32_t b = fpu.regStack[(fpu.status.top + idx) % 8].val;
	fpu.regStack[(fpu.status.top + store_idx) % 8].val = internal_float_add(b, a);
}

void fpu_sub(uint32_t val)
{
	/*
	float *a = (float*)&fpu.regStack[fpu.status.top].val;
	float *b = (float*)&val;
	float c = *a - *b;
	//printf("f %f - %f = %f\n", *a, *b, c);
	uint32_t *d = (uint32_t *)&c;
	fpu.regStack[fpu.status.top].val = *d;
	*/
	fpu.regStack[fpu.status.top].val = internal_float_sub(val, fpu.regStack[fpu.status.top].val);
}

void fpu_mul(uint32_t val)
{
	/*
	float *a = (float*)&fpu.regStack[fpu.status.top].val;
	float *b = (float*)&val;
	float c = *a * *b;
	uint32_t *d = (uint32_t *)&c;
	fpu.regStack[fpu.status.top].val = *d;
	*/
	fpu.regStack[fpu.status.top].val = internal_float_mul(val, fpu.regStack[fpu.status.top].val);
}

void fpu_mul_idx(uint32_t idx, uint32_t store_idx)
{
	/*
	float *a = (float*)&fpu.regStack[fpu.status.top].val;
	float *b = (float*)&fpu.regStack[(fpu.status.top + idx) % 8].val;
	float c = *a * *b;
	uint32_t *d = (uint32_t *)&c;
	fpu.regStack[(fpu.status.top + store_idx) % 8].val = *d;
	*/
	uint32_t a = fpu.regStack[fpu.status.top].val;
	uint32_t b = fpu.regStack[(fpu.status.top + idx) % 8].val;
	fpu.regStack[(fpu.status.top + store_idx) % 8].val = internal_float_mul(b, a);
}

void fpu_div(uint32_t val)
{
	/*
	float *a = (float*)&fpu.regStack[fpu.status.top].val;
	float *b = (float*)&val;
	float c = *a / *b;
	// printf("f %f / %f = %f\n", *a, *b, c);
	uint32_t *d = (uint32_t *)&c;
	fpu.regStack[fpu.status.top].val = *d;
	*/
	fpu.regStack[fpu.status.top].val = internal_float_div(val, fpu.regStack[fpu.status.top].val);
}

void fpu_xch(uint32_t idx)
{
	idx = (fpu.status.top + idx) % 8;
	uint32_t temp = fpu.regStack[fpu.status.top].val;
	fpu.regStack[fpu.status.top].val = fpu.regStack[idx].val;
	fpu.regStack[idx].val = temp;
}

void fpu_copy(uint32_t idx)
{
	idx = (fpu.status.top + idx) % 8;
	fpu.regStack[idx].val = fpu.regStack[fpu.status.top].val;
}

void fpu_cmp(uint32_t idx)
{
	idx = (fpu.status.top + idx) % 8;
	float *a = (float *)&fpu.regStack[fpu.status.top].val;
	float *b = (float *)&fpu.regStack[idx].val;
	if (*a > *b)
	{
		fpu.status.c0 = fpu.status.c2 = fpu.status.c3 = 0;
		// printf("f %f > %f\n", *a, *b);
		// printf("f %x > %x\n", *((uint32_t *)a), *((uint32_t *)b));
	}
	else if (*a < *b)
	{
		fpu.status.c0 = 1;
		fpu.status.c2 = fpu.status.c3 = 0;
		// printf("f %f < %f\n", *a, *b);
	}
	else
	{
		fpu.status.c0 = fpu.status.c2 = 0;
		fpu.status.c3 = 1;
		// printf("f %f == %f\n", *a, *b);
	}
}

void fpu_cmpi(uint32_t idx)
{
	idx = (fpu.status.top + idx) % 8;
	float *a = (float *)&fpu.regStack[fpu.status.top].val;
	float *b = (float *)&fpu.regStack[idx].val;
	if (*a > *b)
	{
		cpu.eflags.ZF = cpu.eflags.PF = cpu.eflags.CF = 0;
	}
	else if (*a < *b)
	{
		cpu.eflags.ZF = cpu.eflags.PF = 0;
		cpu.eflags.CF = 1;
	}
	else
	{
		cpu.eflags.CF = cpu.eflags.PF = 0;
		cpu.eflags.ZF = 1;
	}
}
