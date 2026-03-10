// SPDX-License-Identifier: GPL-2.0
/*
 * RISC-V optimized GHASH routines
 *
 * Copyright (C) 2023 VRULL GmbH
 * Copyright (C) 2024 Siflower Communications Ltd
 * Author: Heiko Stuebner <heiko.stuebner@vrull.eu>
 * 	   Qingfang Deng <qingfang.deng@siflower.com.cn>
 *
 * Based on https://lore.kernel.org/linux-riscv/20230709154243.1582671-4-heiko@sntech.de/ ,
 * but translated to C.
 */

#include <linux/types.h>
#include <linux/err.h>
#include <linux/crypto.h>
#include <linux/module.h>
#include <asm/clmul.h>
#include <asm/unaligned.h>
#include <crypto/ghash.h>
#include <crypto/internal/hash.h>

#define GHASH_MOD_POLY	0xc200000000000000

struct riscv64_ghash_ctx {
	__uint128_t htable;
};

struct riscv64_ghash_desc_ctx {
	__uint128_t shash;
	u8 buffer[GHASH_DIGEST_SIZE];
	int bytes;
};

static int riscv64_ghash_init(struct shash_desc *desc)
{
	struct riscv64_ghash_desc_ctx *dctx = shash_desc_ctx(desc);

	dctx->bytes = 0;
	dctx->shash = 0;
	return 0;
}

/* Compute GMULT (Xi*H mod f) using the Zbc (clmul) extensions.
 * Using the no-Karatsuba approach and clmul for the final reduction.
 * This results in an implementation with minimized number of instructions.
 * HW with clmul latencies higher than 2 cycles might observe a performance
 * improvement with Karatsuba. HW with clmul latencies higher than 6 cycles
 * might observe a performance improvement with additionally converting the
 * reduction to shift&xor. For a full discussion of this estimates see
 * https://github.com/riscv/riscv-crypto/blob/master/doc/supp/gcm-mode-cmul.adoc
 */
static void gcm_ghash_rv64i_zbc(__uint128_t *Xi, __uint128_t k, const u8 *inp, size_t len)
{
	u64 k_hi = k >> 64, k_lo = k, p_hi, p_lo;
	__uint128_t hash = *Xi;

	do {
		__uint128_t t0, t1, t2, t3, lo, mid, hi;

		/* Load the input data, byte-reverse them, and XOR them with Xi */
		p_hi = get_unaligned_be64(inp);
		p_lo = get_unaligned_be64(inp + 8);

		inp += GHASH_BLOCK_SIZE;
		len -= GHASH_BLOCK_SIZE;

		p_hi ^= hash >> 64;
		p_lo ^= hash;

		/* Multiplication (without Karatsuba) */
		t0 = clmul128(p_lo, k_lo);
		t1 = clmul128(p_lo, k_hi);
		t2 = clmul128(p_hi, k_lo);
		t3 = clmul128(p_hi, k_hi);
		mid = t1 ^ t2;
		lo = t0 ^ (mid << 64);
		hi = t3 ^ (mid >> 64);

		/* Reduction with clmul */
		mid = clmul128(lo, GHASH_MOD_POLY);
		lo ^= mid << 64;
		hi ^= lo ^ (mid >> 64);
		hi ^= clmul128(lo >> 64, GHASH_MOD_POLY);
		hash = hi;
	} while (len);

	*Xi = hash;
}

static int riscv64_zbc_ghash_setkey(struct crypto_shash *tfm, const u8 *key, unsigned int keylen)
{
	struct riscv64_ghash_ctx *ctx = crypto_shash_ctx(tfm);
	__uint128_t k;
	u64 hi, lo;

	if (keylen != GHASH_BLOCK_SIZE)
		return -EINVAL;

	hi = get_unaligned_be64(key);
	lo = get_unaligned_be64(key + 8);
	k = (__uint128_t)hi << 64 | lo;
	k = (k << 1 | k >> 127) ^ (k >> 127 ? (__uint128_t)GHASH_MOD_POLY << 64 : 0);
	ctx->htable = k;

	return 0;
}

static int riscv64_zbc_ghash_update(struct shash_desc *desc, const u8 *src, unsigned int srclen)
{
	struct riscv64_ghash_ctx *ctx = crypto_shash_ctx(desc->tfm);
	struct riscv64_ghash_desc_ctx *dctx = shash_desc_ctx(desc);
	unsigned int len;

	if (dctx->bytes) {
		if (dctx->bytes + srclen < GHASH_DIGEST_SIZE) {
			memcpy(dctx->buffer + dctx->bytes, src, srclen);
			dctx->bytes += srclen;
			return 0;
		}
		memcpy(dctx->buffer + dctx->bytes, src, GHASH_DIGEST_SIZE - dctx->bytes);

		gcm_ghash_rv64i_zbc(&dctx->shash, ctx->htable, dctx->buffer, GHASH_DIGEST_SIZE);

		src += GHASH_DIGEST_SIZE - dctx->bytes;
		srclen -= GHASH_DIGEST_SIZE - dctx->bytes;
		dctx->bytes = 0;
	}
	len = srclen & ~(GHASH_DIGEST_SIZE - 1);

	if (len) {
		gcm_ghash_rv64i_zbc(&dctx->shash, ctx->htable, src, len);
		src += len;
		srclen -= len;
	}

	if (srclen) {
		memcpy(dctx->buffer, src, srclen);
		dctx->bytes = srclen;
	}
	return 0;
}

static int riscv64_zbc_ghash_final(struct shash_desc *desc, u8 out[GHASH_DIGEST_SIZE])
{
	struct riscv64_ghash_ctx *ctx = crypto_shash_ctx(desc->tfm);
	struct riscv64_ghash_desc_ctx *dctx = shash_desc_ctx(desc);
	__uint128_t hash;
	int i;

	if (dctx->bytes) {
		for (i = dctx->bytes; i < GHASH_DIGEST_SIZE; i++)
			dctx->buffer[i] = 0;
		gcm_ghash_rv64i_zbc(&dctx->shash, ctx->htable, dctx->buffer, GHASH_DIGEST_SIZE);
		dctx->bytes = 0;
	}
	hash = dctx->shash;
	put_unaligned_be64(hash >> 64, out);
	put_unaligned_be64(hash, out + 8);
	return 0;
}

struct shash_alg riscv64_zbc_ghash_alg = {
	.digestsize = GHASH_DIGEST_SIZE,
	.init = riscv64_ghash_init,
	.update = riscv64_zbc_ghash_update,
	.final = riscv64_zbc_ghash_final,
	.setkey = riscv64_zbc_ghash_setkey,
	.descsize = sizeof(struct riscv64_ghash_desc_ctx),
	.base = {
		 .cra_name = "ghash",
		 .cra_driver_name = "ghash-riscv64-clmul",
		 .cra_priority = 250,
		 .cra_blocksize = GHASH_BLOCK_SIZE,
		 .cra_ctxsize = sizeof(struct riscv64_ghash_ctx),
		 .cra_module = THIS_MODULE,
	},
};

static int __init riscv64_ghash_mod_init(void)
{
	return crypto_register_shash(&riscv64_zbc_ghash_alg);
}

static void __exit riscv64_ghash_mod_fini(void)
{
	crypto_unregister_shash(&riscv64_zbc_ghash_alg);
}

module_init(riscv64_ghash_mod_init);
module_exit(riscv64_ghash_mod_fini);

MODULE_DESCRIPTION("GHASH hash function (CLMUL accelerated)");
MODULE_AUTHOR("Heiko Stuebner <heiko.stuebner@vrull.eu>");
MODULE_LICENSE("GPL");
MODULE_ALIAS_CRYPTO("ghash");
