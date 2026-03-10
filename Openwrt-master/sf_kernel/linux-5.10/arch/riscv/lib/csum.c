// SPDX-License-Identifier: GPL-2.0
/*
 * Checksum library
 *
 * Influenced by arch/arm64/lib/csum.c
 * Copyright (C) 2023-2024 Rivos Inc.
 */
#include <linux/bitops.h>
#include <linux/in6.h>
#include <linux/kasan-checks.h>
#include <linux/kernel.h>

#include <net/checksum.h>

/* Default version is sufficient for 32 bit */
#ifndef CONFIG_32BIT
__sum16 csum_ipv6_magic(const struct in6_addr *saddr,
			const struct in6_addr *daddr,
			__u32 len, __u8 proto, __wsum csum)
{
	unsigned int ulen, uproto;
	unsigned long sum = (__force unsigned long)csum;

	sum += (__force unsigned long)saddr->s6_addr32[0];
	sum += (__force unsigned long)saddr->s6_addr32[1];
	sum += (__force unsigned long)saddr->s6_addr32[2];
	sum += (__force unsigned long)saddr->s6_addr32[3];

	sum += (__force unsigned long)daddr->s6_addr32[0];
	sum += (__force unsigned long)daddr->s6_addr32[1];
	sum += (__force unsigned long)daddr->s6_addr32[2];
	sum += (__force unsigned long)daddr->s6_addr32[3];

	ulen = (__force unsigned int)htonl((unsigned int)len);
	sum += ulen;

	uproto = (__force unsigned int)htonl(proto);
	sum += uproto;

	sum += ror64(sum, 32);
	sum >>= 32;
	return csum_fold((__force __wsum)sum);
}
EXPORT_SYMBOL(csum_ipv6_magic);
#endif /* !CONFIG_32BIT */

#ifdef CONFIG_32BIT
#define OFFSET_MASK 3
#elif CONFIG_64BIT
#define OFFSET_MASK 7
#endif

static inline __no_sanitize_address unsigned long
do_csum_common(const unsigned long *ptr, const unsigned long *end,
	       unsigned long data)
{
	unsigned int shift;
	unsigned long csum = 0, carry = 0;

	/*
	 * Do 32-bit reads on RV32 and 64-bit reads otherwise. This should be
	 * faster than doing 32-bit reads on architectures that support larger
	 * reads.
	 */
	while (ptr < end) {
		csum += data;
		carry += csum < data;
		data = *(ptr++);
	}

	/*
	 * Perform alignment (and over-read) bytes on the tail if any bytes
	 * leftover.
	 */
	shift = ((long)ptr - (long)end) * 8;
#ifdef __LITTLE_ENDIAN
	data = (data << shift) >> shift;
#else
	data = (data >> shift) << shift;
#endif
	csum += data;
	carry += csum < data;
	csum += carry;
	csum += csum < carry;

	return csum;
}

/*
 * Algorithm accounts for buff being misaligned.
 * If buff is not aligned, will over-read bytes but not use the bytes that it
 * shouldn't. The same thing will occur on the tail-end of the read.
 */
static inline __no_sanitize_address unsigned int
do_csum_with_alignment(const unsigned char *buff, int len)
{
	unsigned int offset, shift;
	unsigned long csum, data;
	const unsigned long *ptr, *end;

	/*
	 * Align address to closest word (double word on rv64) that comes before
	 * buff. This should always be in the same page and cache line.
	 * Directly call KASAN with the alignment we will be using.
	 */
	offset = (unsigned long)buff & OFFSET_MASK;
	kasan_check_read(buff, len);
	ptr = (const unsigned long *)(buff - offset);

	/*
	 * Clear the most significant bytes that were over-read if buff was not
	 * aligned.
	 */
	shift = offset * 8;
	data = *(ptr++);
#ifdef __LITTLE_ENDIAN
	data = (data >> shift) << shift;
#else
	data = (data << shift) >> shift;
#endif
	end = (const unsigned long *)(buff + len);
	csum = do_csum_common(ptr, end, data);

#ifndef CONFIG_32BIT
	csum += ror64(csum, 32);
	csum >>= 32;
#endif
	csum = (u32)csum + ror32((u32)csum, 16);
	if (offset & 1)
		return (u16)swab32(csum);
	return csum >> 16;
}

/*
 * Does not perform alignment, should only be used if machine has fast
 * misaligned accesses, or when buff is known to be aligned.
 */
static inline __no_sanitize_address unsigned int
do_csum_no_alignment(const unsigned char *buff, int len)
{
	unsigned long csum, data;
	const unsigned long *ptr, *end;

	ptr = (const unsigned long *)(buff);
	data = *(ptr++);

	kasan_check_read(buff, len);

	end = (const unsigned long *)(buff + len);
	csum = do_csum_common(ptr, end, data);

#ifndef CONFIG_32BIT
	csum += ror64(csum, 32);
	csum >>= 32;
#endif
	csum = (u32)csum + ror32((u32)csum, 16);
	return csum >> 16;
}

/*
 * Perform a checksum on an arbitrary memory address.
 * Will do a light-weight address alignment if buff is misaligned, unless
 * cpu supports fast misaligned accesses.
 */
unsigned int do_csum(const unsigned char *buff, int len)
{
	if (unlikely(len == 0))
		return 0;

	/*
	 * Significant performance gains can be seen by not doing alignment
	 * on machines with fast misaligned accesses.
	 *
	 * There is some duplicate code between the "with_alignment" and
	 * "no_alignment" implmentations, but the overlap is too awkward to be
	 * able to fit in one function without introducing multiple static
	 * branches. The largest chunk of overlap was delegated into the
	 * do_csum_common function.
	 */
	if (IS_ENABLED(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS) ||
	    IS_ALIGNED((unsigned long)buff, sizeof(unsigned long)))
		return do_csum_no_alignment(buff, len);

	return do_csum_with_alignment(buff, len);
}
