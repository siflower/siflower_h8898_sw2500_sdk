// SPDX-License-Identifier: GPL-2.0
#ifndef _RISCV_ASM_CLMUL_H
#define _RISCV_ASM_CLMUL_H

#include <linux/compiler_attributes.h>

static __always_inline __attribute_const__ unsigned long
clmul(unsigned long rs1, unsigned long rs2)
{
	unsigned long rd;

	asm ("clmul	%0, %1, %2" : "=r" (rd) : "r" (rs1), "r" (rs2));
	return rd;
}

static __always_inline __attribute_const__ unsigned long
clmulh(unsigned long rs1, unsigned long rs2)
{
	unsigned long rd;

	asm ("clmulh	%0, %1, %2" : "=r" (rd) : "r" (rs1), "r" (rs2));
	return rd;
}

static __always_inline __attribute_const__ unsigned long
clmulr(unsigned long rs1, unsigned long rs2)
{
	unsigned long rd;

	asm ("clmulr	%0, %1, %2" : "=r" (rd) : "r" (rs1), "r" (rs2));
	return rd;
}

#ifdef CONFIG_64BIT
static __always_inline __uint128_t
clmul128(u64 a, u64 b)
{
	u64 hi, lo;

	asm (
		"clmul	%1, %2, %3\n"
		"clmulh	%0, %2, %3\n"
		: "=&r" (hi), "=&r" (lo) : "r" (a), "r" (b)
	    );
	return (__uint128_t)hi << 64 | lo;
}
#endif

#endif
