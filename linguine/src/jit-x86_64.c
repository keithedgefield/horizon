/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Linguine
 * Copyright (c) 2025, The Linguine Authors. All rights reserved.
 */

/*
 * JIT (x86_64): Just-In-Time native code generation
 */

#include "compat.h"		/* ARCH_X86_64 */

#if defined(ARCH_X86_64) && defined(USE_JIT)

#include "runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if defined(TARGET_WINDOWS)
#include <memoryapi.h>		/* VirtualAlloc(), VirtualProtect(), VirtualFree() */
#else
#include <sys/mman.h>		/* mmap(), mprotect(), munmap() */
#endif

/* False asseretion */
#define JIT_OP_NOT_IMPLEMENTED	0
#define NEVER_COME_HERE		0

/* Error message */
#define BROKEN_BYTECODE		"Broken bytecode."

/* Code size. */
#define CODE_MAX		8192

/* PC entry size. */
#define PC_ENTRY_MAX		2048

/* Branch pathch size. */
#define BRANCH_PATCH_MAX	2048

/* Branch patch type */
#define PATCH_JMP		0
#define PATCH_JE		1
#define PATCH_JNE		2

/* JIT codegen context */
struct jit_context {
	struct rt_env *rt;
	struct rt_func *func;

	/* Generated code. */
	uint8_t *code_top;

	/* Code end. */
	uint8_t *code_end;

	/* Current code position. */
	uint8_t *code;

	/* Exception handler. */
	uint8_t *exception_code;

	/* Current code LIR PC. */
	int lpc;

	/* Table to represent LIR-PC to Arm64-code map. */
	struct pc_entry {
		uint32_t lpc;
		uint8_t *code;
	} pc_entry[PC_ENTRY_MAX];
	int pc_entry_count;

	/* Table to represent branch patching entries. */
	struct branch_patch {
		uint8_t *code;
		uint32_t lpc;
		int type;
	} branch_patch[BRANCH_PATCH_MAX];
	int branch_patch_count;
};

/* Forward declaration */
static bool jit_visit_bytecode(struct jit_context *ctx);
static bool jit_patch_branch(struct jit_context *ctx, int patch_index);

/*
 * Generate a JIT-compiled code for a function.
 */
bool
jit_build(
	  struct rt_env *rt,
	  struct rt_func *func)
{
	struct jit_context ctx;
	int pc;
	int i;

	memset(&ctx, 0, sizeof(struct jit_context));
	ctx.rt = rt;
	ctx.func = func;

	/* Map a new memory region for the generated code. */
#if defined(TARGET_WINDOWS)
	ctx.code_top = VirtualAlloc(NULL, CODE_MAX, MEM_COMMIT, PAGE_READWRITE);
#else
	ctx.code_top = mmap(NULL, CODE_MAX, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
#endif
	if (ctx.code_top == NULL) {
		rt_error(rt, "mmap() failed.");
		return false;
	}
	ctx.code_end = ctx.code_top + CODE_MAX;
	ctx.code = ctx.code_top;

	/* Visit over the bytecode. */
	if (!jit_visit_bytecode(&ctx))
		return false;

	/* Patch branches. */
	for (i = 0; i < ctx.branch_patch_count; i++) {
		if (!jit_patch_branch(&ctx, i))
			return false;
	}

	/* Make code executable and non-writable. */
#if defined(TARGET_WINDOWS)
	DWORD dwOldProt;
	VirtualProtect(ctx.code_top, CODE_MAX, PAGE_EXECUTE_READ, &dwOldProt);
#else
	mprotect(ctx.code_top, CODE_MAX, PROT_EXEC | PROT_READ);
#endif

	func->jit_code = (bool (*)(struct rt_env *))ctx.code_top;

	return true;
}

/*
 * Free a JIT-compiled code for a function.
 */
void
jit_free(
	 struct rt_env *rt,
	 struct rt_func *func)
{
#if defined(TARGET_WINDOWS)
	VirtualFree(func->jit_code, CODE_MAX);
#else
	munmap(func->jit_code, CODE_MAX);
#endif
}

/*
 * Assembler output functions
 */

/* Serif */
#define ASM

/* Put a instruction byte. */
#define IB(b)			if (!jit_put_byte(ctx, b)) return false
static INLINE bool
jit_put_byte(
	struct jit_context *ctx,
	uint8_t b)
{
	if (ctx->code + 1 > ctx->code_end) {
		rt_error(ctx->rt, "Code too big.");
		return false;
	}

	*ctx->code++ = b;

	return true;
}

/* Put a instruction double word. */
#define ID(d)			if (!jit_put_dword(ctx, d)) return false
static INLINE bool
jit_put_dword(
	struct jit_context *ctx,
	uint32_t dw)
{
	if (ctx->code + 4 > ctx->code_end) {
		rt_error(ctx->rt, "Code too big.");
		return false;
	}

	*ctx->code++ = (uint8_t)(dw & 0xff);
	*ctx->code++ = (uint8_t)((dw >> 8) & 0xff);
	*ctx->code++ = (uint8_t)((dw >> 16) & 0xff);
	*ctx->code++ = (uint8_t)((dw >> 24) & 0xff);

	return true;
}

/* Put a instruction word. */
#define IQ(q)			if (!jit_put_qword(ctx, q)) return false
static INLINE bool
jit_put_qword(
	struct jit_context *ctx,
	uint64_t qw)
{
	if (ctx->code + 8 > ctx->code_end) {
		rt_error(ctx->rt, "Code too big.");
		return false;
	}

	*ctx->code++ = (uint8_t)(qw & 0xff);
	*ctx->code++ = (uint8_t)((qw >> 8) & 0xff);
	*ctx->code++ = (uint8_t)((qw >> 16) & 0xff);
	*ctx->code++ = (uint8_t)((qw >> 24) & 0xff);
	*ctx->code++ = (uint8_t)((qw >> 32) & 0xff);
	*ctx->code++ = (uint8_t)((qw >> 40) & 0xff);
	*ctx->code++ = (uint8_t)((qw >> 48) & 0xff);
	*ctx->code++ = (uint8_t)((qw >> 56) & 0xff);

	return true;
}

/*
 * Bytecode getter
 */

/* Check an opcode. */
#define CONSUME_OPCODE(d)		if (!jit_get_opcode(ctx, &d)) return false
static INLINE bool
jit_get_opcode(
	struct jit_context *ctx,
	uint8_t *opcode)
{
	if (ctx->lpc + 1 > ctx->func->bytecode_size) {
		rt_error(ctx->rt, BROKEN_BYTECODE);
		return false;
	}

	*opcode = ctx->func->bytecode[ctx->lpc];

	ctx->lpc++;

	return true;
}

/* Get a imm32 operand. */
#define CONSUME_IMM32(d)		if (!jit_get_opr_imm32(ctx, &d)) return false
static INLINE bool
jit_get_opr_imm32(
	struct jit_context *ctx,
	uint32_t *d)
{
	if (ctx->lpc + 4 > ctx->func->bytecode_size) {
		rt_error(ctx->rt, BROKEN_BYTECODE);
		return false;
	}

	*d = (ctx->func->bytecode[ctx->lpc] << 24) |
	     (ctx->func->bytecode[ctx->lpc + 1] << 16) |
	     (ctx->func->bytecode[ctx->lpc + 2] << 8) |
	      ctx->func->bytecode[ctx->lpc + 3];

	ctx->lpc += 4;

	return true;
}

/* Get an imm16 operand that represents tmpvar index. */
#define CONSUME_TMPVAR(d)		if (!jit_get_opr_tmpvar(ctx, &d)) return false
static INLINE bool
jit_get_opr_tmpvar(
	struct jit_context *ctx,
	int *d)
{
	if (ctx->lpc + 2 > ctx->func->bytecode_size) {
		rt_error(ctx->rt, BROKEN_BYTECODE);
		return false;
	}

	*d = (ctx->func->bytecode[ctx->lpc] << 8) |
	      ctx->func->bytecode[ctx->lpc + 1];
	if (*d >= ctx->func->tmpvar_size) {
		rt_error(ctx->rt, BROKEN_BYTECODE);
		return false;
	}

	ctx->lpc += 2;

	return true;
}

/* Check an opcode. */
#define CONSUME_IMM8(d)		if (!jit_get_imm8(ctx, &d)) return false
static INLINE bool
jit_get_imm8(
	struct jit_context *ctx,
	int *imm8)
{
	if (ctx->lpc + 1 > ctx->func->bytecode_size) {
		rt_error(ctx->rt, BROKEN_BYTECODE);
		return false;
	}

	*imm8 = ctx->func->bytecode[ctx->lpc];

	ctx->lpc++;

	return true;
}

/* Get a string operand. */
#define CONSUME_STRING(d)		if (!jit_get_opr_string(ctx, &d)) return false
static INLINE bool
jit_get_opr_string(
	struct jit_context *ctx,
	const char **d)
{
	int len;

	len = strlen((const char *)&ctx->func->bytecode[ctx->lpc]);
	if (ctx->lpc + len + 1 > ctx->func->bytecode_size) {
		rt_error(ctx->rt, BROKEN_BYTECODE);
		return false;
	}

	*d = (const char *)&ctx->func->bytecode[ctx->lpc];

	ctx->lpc += len + 1;

	return true;
}

/*
 * Templates
 */

#define ASM_BINARY_OP(f)											\
	/* if (!f(rt, dst, src1, src2)) return false; */							\
	ASM {													\
		/* r13: exception_handler */									\
		/* r14: rt */											\
		/* r14: &rt->frame->tmpvar[0] */								\
														\
		/* movq %r14, %rdi */			IB(0x4c); IB(0x89); IB(0xf7);				\
		/* movq dst, %rsi */			IB(0x48); IB(0xc7); IB(0xc6); ID(dst); 			\
		/* movq src1, %rdx */			IB(0x48); IB(0xc7); IB(0xc2); ID(src1);			\
		/* movq src2, %rcx */			IB(0x48); IB(0xc7); IB(0xc1); ID(src2);			\
		/* movabs f, %r8 */			IB(0x49); IB(0xb8); IQ((uint64_t)f);			\
		/* call *%r8 */				IB(0x41); IB(0xff); IB(0xd0);				\
														\
		/* cmpl $0, %eax */	IB(0x83); IB(0xf8); IB(0x00);						\
		/* jne 8 <next> */	IB(0x75); IB(0x03);							\
		/* jmp *%r13 */		IB(0x41); IB(0xff); IB(0xe5);						\
		/* next:*/											\
	}

#define ASM_UNARY_OP(f)												\
	/* if (!f(rt, dst, src)) return false; */								\
	ASM {													\
		/* r13: exception_handler */									\
		/* r14: rt */											\
		/* r14: &rt->frame->tmpvar[0] */								\
														\
		/* movq %r14, %rdi */			IB(0x4c); IB(0x89); IB(0xf7);				\
		/* movq dst, %rsi */			IB(0x48); IB(0xc7); IB(0xc6); ID(dst); 			\
		/* movq src, %rdx */			IB(0x48); IB(0xc7); IB(0xc2); ID(src);			\
		/* movabs f, %r8 */			IB(0x49); IB(0xb8); IQ((uint64_t)f);			\
		/* call *%r8 */				IB(0x41); IB(0xff); IB(0xd0);				\
														\
		/* cmpl $0, %eax */	IB(0x83); IB(0xf8); IB(0x00);						\
		/* jne 8 <next> */	IB(0x75); IB(0x03);							\
		/* jmp *%r13 */		IB(0x41); IB(0xff); IB(0xe5);						\
		/* next:*/											\
	}

/*
 * Bytecode visitors
 */

/* Visit a ROP_LINEINFO instruction. */
static INLINE bool
jit_visit_lineinfo_op(
	struct jit_context *ctx)
{
	uint32_t line;
	uint64_t addr;

	CONSUME_IMM32(line);

	/* rt->line = line; */
	ASM {
		/* r14 = rt */

		/* movl line, %rax */		IB(0x48); IB(0xc7); IB(0xc0); ID(line);
		/* movq %rax, 0x8(%r14) */	IB(0x49); IB(0x89); IB(0x46); IB(0x08);
	}

	return true;
}

/* Visit a ROP_ASSIGN instruction. */
static INLINE bool
jit_visit_assign_op(
	struct jit_context *ctx)
{
	int pc;
	int dst;
	int src;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src);

	/* rt->frame->tmpvar[dst] = rt->frame->tmpvar[src]; */
	ASM {
		/* r15 = &rt->frame->tmpvar[0] */

		/* movq dst, %rax */		IB(0x48); IB(0xc7); IB(0xc0); ID(dst);
		/* movq src, %rbx */		IB(0x48); IB(0xc7); IB(0xc3); ID(src);
		/* shlq $4, %rax */		IB(0xc1); IB(0xe0); IB(0x04);
		/* shlq $4, %rbx */		IB(0xc1); IB(0xe3); IB(0x04);
		/* addq %r15, %rax */		IB(0x4c); IB(0x01); IB(0xf8);
		/* addq %r15, %rbx */		IB(0x4c); IB(0x01); IB(0xfb);
		/* movq (%rbx), %rcx */		IB(0x48); IB(0x8b); IB(0x0b);
		/* movq 8(%rbx), %rdx */	IB(0x48); IB(0x8b); IB(0x53); IB(0x08);
		/* movq %rcx, (%rax) */		IB(0x48); IB(0x89); IB(0x08);
		/* movq %rdx, 8(%rax) */	IB(0x48); IB(0x89); IB(0x50); IB(0x08);
	}

	return true;
}

/* Visit a ROP_ICONST instruction. */
static INLINE bool
jit_visit_iconst_op(
	struct jit_context *ctx)
{
	int dst;
	uint32_t val;

	CONSUME_TMPVAR(dst);
	CONSUME_IMM32(val);

	/* &rt->frame->tmpvar[dst].type = RT_VALUE_INT; */
	/* &rt->frame->tmpvar[dst].val.i = val; */
	ASM {
		/* r15 = &rt->frame->tmpvar[0] */

		/* movq dst, %rax */		IB(0x48); IB(0xc7); IB(0xc0); ID(dst);
		/* shlq $4, %rax */		IB(0x48); IB(0xc1); IB(0xe0); IB(04);
		/* addq %r15, %rax */		IB(0x4c); IB(0x01); IB(0xf8);
		/* movl $0, (%rax) */		IB(0xc7); IB(0x00); ID(0);
		/* movl val, 8(%rax) */		IB(0xc7); IB(0x40); IB(0x08); ID(val);
	}

	return true;
}

/* Visit a ROP_FCONST instruction. */
static INLINE bool
jit_visit_fconst_op(
	struct jit_context *ctx)
{
	int dst;
	uint32_t val;

	CONSUME_TMPVAR(dst);
	CONSUME_IMM32(val);

	/* &rt->frame->tmpvar[dst].type = RT_VALUE_INT; */
	/* &rt->frame->tmpvar[dst].val.i = val; */
	ASM {
		/* r15 = &rt->frame->tmpvar[0] */

		/* movq dst, %rax */		IB(0x48); IB(0xc7); IB(0xc0); ID(dst);
		/* shlq $4, %rax */		IB(0x48); IB(0xc1); IB(0xe0); IB(04);
		/* addq %r15, %rax */		IB(0x4c); IB(0x01); IB(0xf8);
		/* movl $1, (%rax) */		IB(0xc7); IB(0x00); ID(1);
		/* movl val, 8(%rax) */		IB(0xc7); IB(0x40); IB(0x08); ID(val);
	}

	return true;
}

/* Visit a ROP_SCONST instruction. */
static INLINE bool
jit_visit_sconst_op(
	struct jit_context *ctx)
{
	int dst;
	const char *val;

	CONSUME_TMPVAR(dst);
	CONSUME_STRING(val);

	/* rt_make_string(rt, &rt->frame->tmpvar[dst], val); */
	ASM {
		/* r13 = exception_handler */
		/* r14 = rt */
		/* r15 = &rt->frame->tmpvar[0] */

		/* movq %r14, %rdi */			IB(0x4c); IB(0x89); IB(0xf7);
		/* movq dst, %rsi */			IB(0x48); IB(0xc7); IB(0xc6); ID(dst);
		/* shlq $4, %rsi */			IB(0x48); IB(0xc1); IB(0xe6); IB(0x04);
		/* addq %r15, %rsi */			IB(0x4c); IB(0x01); IB(0xfe);
		/* movabs val, %rdx */			IB(0x48); IB(0xba); IQ((uint64_t)val);
		/* movabs rt_make_string, %r8 */	IB(0x49); IB(0xb8); IQ((uint64_t)rt_make_string);
		/* call *%r8 */				IB(0x41); IB(0xff); IB(0xd0);

		/* cmpl $0, %eax */			IB(0x83); IB(0xf8); IB(0x00);
		/* jne 8 <next> */			IB(0x75); IB(0x03);
		/* jmp *%r13 */				IB(0x41); IB(0xff); IB(0xe5);
	/* next: */
	}

	return true;
}

/* Visit a ROP_ACONST instruction. */
static INLINE bool
jit_visit_aconst_op(
	struct jit_context *ctx)
{
	int dst;

	CONSUME_TMPVAR(dst);

	/* rt_make_empty_array(rt, &rt->frame->tmpvar[dst]); */
	ASM {
		/* movq %r14, %rdi */			IB(0x4c); IB(0x89); IB(0xf7);
		/* movq dst, %rsi */			IB(0x48); IB(0xc7); IB(0xc6); ID(dst);
		/* shlq $4, %rsi */			IB(0x48); IB(0xc1); IB(0xe6); IB(0x04);
		/* addq %r15, %rsi */			IB(0x4c); IB(0x01); IB(0xfe);
		/* movabs rt_make_empty_array, %r8 */	IB(0x49); IB(0xb8); IQ((uint64_t)rt_make_empty_array);
		/* call *%r8 */				IB(0x41); IB(0xff); IB(0xd0);

		/* cmpl $0, %eax */			IB(0x83); IB(0xf8); IB(0x00);
		/* jne 8 <next> */			IB(0x75); IB(0x03);
		/* jmp *%r13 */				IB(0x41); IB(0xff); IB(0xe5);
	/* next: */
	}

	return true;
}

/* Visit a ROP_DCONST instruction. */
static INLINE bool
jit_visit_dconst_op(
	struct jit_context *ctx)
{
	int dst;

	CONSUME_TMPVAR(dst);

	/* rt_make_empty_dict(rt, &rt->frame->tmpvar[dst]); */
	ASM {
		/* movq %r14, %rdi */			IB(0x4c); IB(0x89); IB(0xf7);
		/* movq dst, %rsi */			IB(0x48); IB(0xc7); IB(0xc6); ID(dst);
		/* shlq $4, %rsi */			IB(0x48); IB(0xc1); IB(0xe6); IB(0x04);
		/* addq %r15, %rsi */			IB(0x4c); IB(0x01); IB(0xfe);
		/* movabs rt_make_empty_dict, %r8 */	IB(0x49); IB(0xb8); IQ((uint64_t)rt_make_empty_dict);
		/* call *%r8 */				IB(0x41); IB(0xff); IB(0xd0);

		/* cmpl $0, %eax */			IB(0x83); IB(0xf8); IB(0x00);
		/* jne 8 <next> */			IB(0x75); IB(0x03);
		/* jmp *%r13 */				IB(0x41); IB(0xff); IB(0xe5);
	/* next: */
	}

	return true;
}

/* Visit a ROP_INC instruction. */
static INLINE bool
jit_visit_inc_op(
	struct jit_context *ctx)
{
	struct rt_value *val;
	int dst;

	CONSUME_TMPVAR(dst);

	/* &rt->frame->tmpvar[dst].val.i++ */
	ASM {
		/* r15 = &rt->frame->tmpvar[0] */

		/* movq dst, %rax */			IB(0x48); IB(0xc7); IB(0xc0); ID(dst);
		/* shlq $4, %rax */			IB(0x48); IB(0xc1); IB(0xe0); IB(0x04);
		/* addq %r15, %rax */			IB(0x4c); IB(0x01); IB(0xf8);
		/* incq 8(%rax) */			IB(0x48); IB(0xff); IB(0x40); IB(0x08);
	}

	return true;
}

/* Visit a ROP_ADD instruction. */
static INLINE bool
jit_visit_add_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_add_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_add_helper);

	return true;
}

/* Visit a ROP_SUB instruction. */
static INLINE bool
jit_visit_sub_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_sub_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_sub_helper);

	return true;
}

/* Visit a ROP_MUL instruction. */
static INLINE bool
jit_visit_mul_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_mul_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_mul_helper);

	return true;
}

/* Visit a ROP_DIV instruction. */
static INLINE bool
jit_visit_div_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_div_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_div_helper);

	return true;
}

/* Visit a ROP_MOD instruction. */
static INLINE bool
jit_visit_mod_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_mod_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_mod_helper);

	return true;
}

/* Visit a ROP_AND instruction. */
static INLINE bool
jit_visit_and_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_and_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_and_helper);

	return true;
}

/* Visit a ROP_OR instruction. */
static INLINE bool
jit_visit_or_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_or_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_or_helper);

	return true;
}

/* Visit a ROP_XOR instruction. */
static INLINE bool
jit_visit_xor_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_xor_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_xor_helper);

	return true;
}

/* Visit a ROP_XOR instruction. */
static INLINE bool
jit_visit_neg_op(
	struct jit_context *ctx)
{
	int dst;
	int src;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src);

	/* if (!jit_neg_helper(rt, dst, src)) return false; */
	ASM_UNARY_OP(rt_xor_helper);

	return true;
}

/* Visit a ROP_LT instruction. */
static INLINE bool
jit_visit_lt_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_lt_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_lt_helper);

	return true;
}

/* Visit a ROP_LTE instruction. */
static INLINE bool
jit_visit_lte_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_lte_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_lte_helper);

	return true;
}

/* Visit a ROP_EQ instruction. */
static INLINE bool
jit_visit_eq_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_eq_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_eq_helper);

	return true;
}

/* Visit a ROP_NEQ instruction. */
static INLINE bool
jit_visit_neq_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_neq_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_neq_helper);

	return true;
}

/* Visit a ROP_GTE instruction. */
static INLINE bool
jit_visit_gte_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_gte_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_gte_helper);

	return true;
}

/* Visit a ROP_EQI instruction. */
static INLINE bool
jit_visit_eqi_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* src1 - src2 */
	ASM {
		/* movq dst, %rax */		IB(0x48); IB(0xc7); IB(0xc0); ID(dst);
		/* shlq $4, %rax */		IB(0x48); IB(0xc1); IB(0xe0); IB(0x04);
		/* addq %r15, %rax */		IB(0x4c); IB(0x01); IB(0xf8);

		/* movq src1, %rbx */		IB(0x48); IB(0xc7); IB(0xc3); ID(src1);
		/* shlq $4, %rbx */		IB(0x48); IB(0xc1); IB(0xe3); IB(0x04);
		/* addq %r15, %rbx */		IB(0x4c); IB(0x01); IB(0xfb);
		
		/* movq src2, %rcx */		IB(0x48); IB(0xc7); IB(0xc1); ID(src2);
		/* shlq $4, %rcx */		IB(0x48); IB(0xc1); IB(0xe1); IB(0x04);
		/* addq %r15, %rcx */		IB(0x4c); IB(0x01); IB(0xf9);

		/* movq 8(%rbx), %rax */	IB(0x48); IB(0x8b); IB(0x43); IB(0x08);
		/* movq 8(%rcx), %rdx */	IB(0x48); IB(0x8b); IB(0x51); IB(0x08);
		/* cmpl %eax, %edx */		IB(0x39); IB(0xc2);
	}

	return true;
}

/* Visit a ROP_GT instruction. */
static INLINE bool
jit_visit_gt_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_gt_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_gt_helper);

	return true;
}

/* Visit a ROP_LOADARRAY instruction. */
static INLINE bool
jit_visit_loadarray_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_loadarray_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_loadarray_helper);

	return true;
}

/* Visit a ROP_STOREARRAY instruction. */
static INLINE bool
jit_visit_storearray_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_storearray_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_storearray_helper);

	return true;
}

/* Visit a ROP_LEN instruction. */
static INLINE bool
jit_visit_len_op(
	struct jit_context *ctx)
{
	int dst;
	int src;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src);

	/* if (!jit_len_helper(rt, dst, src)) return false; */
	ASM_UNARY_OP(rt_len_helper);

	return true;
}

/* Visit a ROP_GETDICTKEYBYINDEX instruction. */
static INLINE bool
jit_visit_getdictkeybyindex_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_getdictkeybyindex_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_getdictkeybyindex_helper);

	return true;
}

/* Visit a ROP_GETDICTVALBYINDEX instruction. */
static INLINE bool
jit_visit_getdictvalbyindex_op(
	struct jit_context *ctx)
{
	int dst;
	int src1;
	int src2;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(src1);
	CONSUME_TMPVAR(src2);

	/* if (!jit_getdictvalbyindex_helper(rt, dst, src1, src2)) return false; */
	ASM_BINARY_OP(rt_getdictvalbyindex_helper);

	return true;
}

/* Visit a ROP_LOADSYMBOL instruction. */
static INLINE bool
jit_visit_loadsymbol_op(
	struct jit_context *ctx)
{
	int dst;
	const char *src_s;
	uint64_t src;

	CONSUME_TMPVAR(dst);
	CONSUME_STRING(src_s);
	src = (intptr_t)src_s;

	/* if (!rt_loadsymbol_helper(rt, dst, src)) return false; */
	ASM {
		/* r13: exception_handler */
		/* r14: rt */
		/* r14: &rt->frame->tmpvar[0] */

		/* movq %r14, %rdi */			IB(0x4c); IB(0x89); IB(0xf7);
		/* movq dst, %rsi */			IB(0x48); IB(0xc7); IB(0xc6); ID(dst);
		/* movabs src, %rdx */			IB(0x48); IB(0xba); IQ(src);
		/* movabs rt_loadsymbol_helper, %r8 */	IB(0x49); IB(0xb8); IQ((uint64_t)rt_loadsymbol_helper);
		/* call *%r8 */				IB(0x41); IB(0xff); IB(0xd0);

		/* cmpl $0, %eax */			IB(0x83); IB(0xf8); IB(0x00);
		/* jne 8 <next> */			IB(0x75); IB(0x03);
		/* jmp *%r13 */				IB(0x41); IB(0xff); IB(0xe5);
		/* next:*/
	}

	return true;
}

/* Visit a ROP_STORESYMBOL instruction. */
static INLINE bool
jit_visit_storesymbol_op(
	struct jit_context *ctx)
{
	const char *dst_s;
	uint64_t dst;
	int src;

	CONSUME_STRING(dst_s);
	CONSUME_TMPVAR(src);
	dst = (intptr_t)dst_s;

	/* if (!rt_storesymbol_helper(rt, dst, src)) return false; */
	ASM {
		/* r13: exception_handler */
		/* r14: rt */
		/* r14: &rt->frame->tmpvar[0] */

		/* movq %r14, %rdi */			IB(0x4c); IB(0x89); IB(0xf7);
		/* movabs dst, %rsi */			IB(0x48); IB(0xbe); IQ(dst);
		/* movq src, %rdx */			IB(0x48); IB(0xc7); IB(0xc2); ID(src);
		/* movabs rt_storesymbol_helper, %r8 */	IB(0x49); IB(0xb8); IQ((uint64_t)rt_storesymbol_helper);
		/* call *%r8 */				IB(0x41); IB(0xff); IB(0xd0);

		/* cmpl $0, %eax */			IB(0x83); IB(0xf8); IB(0x00);
		/* jne 8 <next> */			IB(0x75); IB(0x03);
		/* jmp *%r13 */				IB(0x41); IB(0xff); IB(0xe5);
		/* next:*/
	}

	return true;
}

/* Visit a ROP_LOADDOT instruction. */
static INLINE bool
jit_visit_loaddot_op(
	struct jit_context *ctx)
{
	int dst;
	int dict;
	const char *field_s;
	uint64_t field;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(dict);
	CONSUME_STRING(field_s);
	field = (intptr_t)field_s;

	/* if (!rt_loaddot_helper(rt, dst, dict, field)) return false; */
	ASM {
		/* r13: exception_handler */
		/* r14: rt */
		/* r14: &rt->frame->tmpvar[0] */

		/* movq %r14, %rdi */			IB(0x4c); IB(0x89); IB(0xf7);
		/* movq dst, %rsi */			IB(0x48); IB(0xc7); IB(0xc6); ID(dst);
		/* movq dict, %rdx */			IB(0x48); IB(0xc7); IB(0xc2); ID(dict);
		/* movabs field, %rcx */		IB(0x48); IB(0xb9); IQ(field);
		/* movabs rt_loaddot_helper, %r8 */	IB(0x49); IB(0xb8); IQ((uint64_t)rt_loaddot_helper);
		/* call *%r8 */				IB(0x41); IB(0xff); IB(0xd0);

		/* cmpl $0, %eax */			IB(0x83); IB(0xf8); IB(0x00);
		/* jne 8 <next> */			IB(0x75); IB(0x03);
		/* jmp *%r13 */				IB(0x41); IB(0xff); IB(0xe5);
		/* next:*/
	}

	return true;
}

/* Visit a ROP_STOREDOT instruction. */
static INLINE bool
jit_visit_storedot_op(
	struct jit_context *ctx)
{
	int dict;
	const char *field_s;
	uint64_t field;
	int src;

	CONSUME_TMPVAR(dict);
	CONSUME_STRING(field_s);
	CONSUME_TMPVAR(src);
	field = (intptr_t)field_s;

	/* if (!jit_storedot_helper(rt, dict, field, src)) return false; */
	ASM {
		/* r13: exception_handler */
		/* r14: rt */
		/* r14: &rt->frame->tmpvar[0] */

		/* movq %r14, %rdi */			IB(0x4c); IB(0x89); IB(0xf7);
		/* movq dict, %rsi */			IB(0x48); IB(0xc7); IB(0xc6); ID(dict);
		/* movabs field, %rdx */		IB(0x48); IB(0xba); IQ(field);
		/* movq src, %rcx */			IB(0x48); IB(0xc7); IB(0xc1); ID(src);
		/* movabs rt_storedot_helper, %r8 */	IB(0x49); IB(0xb8); IQ((uint64_t)rt_storedot_helper);
		/* call *%r8 */				IB(0x41); IB(0xff); IB(0xd0);

		/* cmpl $0, %eax */			IB(0x83); IB(0xf8); IB(0x00);
		/* jne 8 <next> */			IB(0x75); IB(0x03);
		/* jmp *%r13 */				IB(0x41); IB(0xff); IB(0xe5);
		/* next:*/
	}

	return true;
}

/* Visit a ROP_CALL instruction. */
static inline bool
jit_visit_call_op(
	struct jit_context *ctx)
{
	int dst;
	int func;
	int arg_count;
	int arg_tmp;
	int arg[RT_ARG_MAX];
	uint64_t arg_addr;
	int i;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(func);
	CONSUME_IMM8(arg_count);
	for (i = 0; i < arg_count; i++) {
		CONSUME_TMPVAR(arg_tmp);
		arg[i] = arg_tmp;
	}

	/* Embed arguments to the code. */
	ASM {
		/* jmp (5 + arg_count * 4) */
		IB(0xe9);
		ID(4 * arg_count);
	}
	arg_addr = (intptr_t)ctx->code;
	for (i = 0; i < arg_count; i++) {
		*(int *)ctx->code = (uint32_t)arg[i];
		ctx->code += 4;
	}

	/* if (!rt_call_helper(rt, dst, func, arg_count, arg)) return false; */
	ASM {
		/* r13: exception_handler */
		/* r14: rt */
		/* r14: &rt->frame->tmpvar[0] */

		/* movq %r14, %rdi */			IB(0x4c); IB(0x89); IB(0xf7);
		/* movq dst, %rsi */			IB(0x48); IB(0xc7); IB(0xc6); ID(dst);
		/* movq func, %rdx */			IB(0x48); IB(0xc7); IB(0xc2); ID(func);
		/* movq arg_count, %rcx */		IB(0x48); IB(0xc7); IB(0xc1); ID(arg_count);
		/* movabs arg_addr, %r8 */		IB(0x49); IB(0xb8); IQ(arg_addr);
		/* movabs rt_call_helper, %r9 */	IB(0x49); IB(0xb9); IQ((uint64_t)rt_call_helper);
		/* call *%r9 */				IB(0x41); IB(0xff); IB(0xd1);

		/* cmpl $0, %eax */			IB(0x83); IB(0xf8); IB(0x00);
		/* jne 8 <next> */			IB(0x75); IB(0x03);
		/* jmp *%r13 */				IB(0x41); IB(0xff); IB(0xe5);
		/* next:*/
	}
	
	return true;
}

/* Visit a ROP_THISCALL instruction. */
static inline bool
jit_visit_thiscall_op(
	struct jit_context *ctx)
{
	int dst;
	int obj;
	const char *symbol;
	int arg_count;
	int arg_tmp;
	int arg[RT_ARG_MAX];
	uint64_t arg_addr;
	int i;

	CONSUME_TMPVAR(dst);
	CONSUME_TMPVAR(obj);
	CONSUME_STRING(symbol);
	CONSUME_IMM8(arg_count);
	for (i = 0; i < arg_count; i++) {
		CONSUME_TMPVAR(arg_tmp);
		arg[i] = arg_tmp;
	}

	/* Embed arguments to the code. */
	ASM {
		/* jmp (5 + arg_count * 4) */
		IB(0xe9);
		ID(4 * arg_count);
	}
	arg_addr = (intptr_t)ctx->code;
	for (i = 0; i < arg_count; i++) {
		*(int *)ctx->code = (uint32_t)arg[i];
		ctx->code += 4;
	}

	/* if (!rt_thiscall_helper(rt, dst, obj, symbol, arg_count, arg)) return false; */
	ASM {
		/* r13: exception_handler */
		/* r14: rt */
		/* r14: &rt->frame->tmpvar[0] */

		/* movq %r14, %rdi */			IB(0x4c); IB(0x89); IB(0xf7);
		/* movq dst, %rsi */			IB(0x48); IB(0xc7); IB(0xc6); ID(dst);
		/* movq obj, %rdx */			IB(0x48); IB(0xc7); IB(0xc2); ID(obj);
		/* movabs symbol, %rcx */		IB(0x48); IB(0x89); IQ((uint64_t)symbol);
		/* movq arg_count, %r8 */		IB(0x49); IB(0xc7); IB(0xc0); ID(arg_count);
		/* movabs arg_addr, %r9 */		IB(0x49); IB(0xb9); IQ(arg_addr);
		/* movabs rt_call_helper, %r10 */	IB(0x49); IB(0xba); IQ((uint64_t)rt_call_helper);
		/* call *%r10 */			IB(0x41); IB(0xff); IB(0xd2);

		/* cmpl $0, %eax */			IB(0x83); IB(0xf8); IB(0x00);
		/* jne 8 <next> */			IB(0x75); IB(0x03);
		/* jmp *%r13 */				IB(0x41); IB(0xff); IB(0xe5);
		/* next:*/
	}

	return true;
}

/* Visit a ROP_JMP instruction. */
static inline bool
jit_visit_jmp_op(
	struct jit_context *ctx)
{
	uint32_t target_lpc;

	CONSUME_IMM32(target_lpc);
	if (target_lpc >= ctx->func->bytecode_size + 1) {
		rt_error(ctx->rt, BROKEN_BYTECODE);
		return false;
	}

	/* Patch later. */
	ctx->branch_patch[ctx->branch_patch_count].code = ctx->code;
	ctx->branch_patch[ctx->branch_patch_count].lpc = target_lpc;
	ctx->branch_patch[ctx->branch_patch_count].type = PATCH_JMP;
	ctx->branch_patch_count++;

	ASM {
		/* Patched later. */
		/* jmp 5 */	IB(0xe9); ID(0);
	}

	return true;
}

/* Visit a ROP_JMPIFTRUE instruction. */
static inline bool
jit_visit_jmpiftrue_op(
	struct jit_context *ctx)
{
	int src;
	uint32_t target_lpc;

	CONSUME_TMPVAR(src);
	CONSUME_IMM32(target_lpc);
	if (target_lpc >= ctx->func->bytecode_size + 1) {
		rt_error(ctx->rt, BROKEN_BYTECODE);
		return false;
	}

	ASM {
		/* rdx = &rt->frame->tmpvar[src] */
		/* movq %r14, %rcx */			IB(0x4c); IB(0x89); IB(0xf1);
		/* movq src, %rdx */			IB(0x48); IB(0xc7); IB(0xc2); ID(src);
		/* shlq $4, %rdx */			IB(0x48); IB(0xc1); IB(0xe2); IB(0x04);
		/* addq %r15, %rdx */			IB(0x4c); IB(0x01); IB(0xfa);
		/* movl 8(%rdx), %eax */		IB(0x8b); IB(0x42); IB(0x08);

		/* Compare: rt->frame->tmpvar[dst].val.i == 1 */
		/* cmpl $0, %eax */			IB(0x83); IB(0xf8); IB(0x00);
	}
	
	/* Patch later. */
	ctx->branch_patch[ctx->branch_patch_count].code = ctx->code;
	ctx->branch_patch[ctx->branch_patch_count].lpc = target_lpc;
	ctx->branch_patch[ctx->branch_patch_count].type = PATCH_JNE;
	ctx->branch_patch_count++;

	ASM {
		/* Patched later. */
		/* jne 6 */				IB(0x0f); IB(0x84); ID(0);
	}

	return true;
}

/* Visit a ROP_JMPIFFALSE instruction. */
static inline bool
jit_visit_jmpiffalse_op(
	struct jit_context *ctx)
{
	int src;
	uint32_t target_lpc;

	CONSUME_TMPVAR(src);
	CONSUME_IMM32(target_lpc);
	if (target_lpc >= ctx->func->bytecode_size + 1) {
		rt_error(ctx->rt, BROKEN_BYTECODE);
		return false;
	}

	ASM {
		/* rdx = &rt->frame->tmpvar[src] */
		/* movq %r14, %rcx */			IB(0x4c); IB(0x89); IB(0xf1);
		/* movq src, %rdx */			IB(0x48); IB(0xc7); IB(0xc2); ID(src);
		/* shlq $4, %rdx */			IB(0x48); IB(0xc1); IB(0xe2); IB(0x04);
		/* addq %r15, %rdx */			IB(0x4c); IB(0x01); IB(0xfa);
		/* movl 8(%rdx), %eax */		IB(0x8b); IB(0x42); IB(0x08);

		/* Compare: rt->frame->tmpvar[dst].val.i == 1 */
		/* cmpl $0, %eax */			IB(0x83); IB(0xf8); IB(0x00);
	}
	
	/* Patch later. */
	ctx->branch_patch[ctx->branch_patch_count].code = ctx->code;
	ctx->branch_patch[ctx->branch_patch_count].lpc = target_lpc;
	ctx->branch_patch[ctx->branch_patch_count].type = PATCH_JE;
	ctx->branch_patch_count++;

	ASM {
		/* Patched later. */
		/* je 6 */				IB(0x0f); IB(0x85); ID(0);
	}

	return true;
}

/* Visit a ROP_JMPIFEQ instruction. */
static inline bool
jit_visit_jmpifeq_op(
	struct jit_context *ctx)
{
	int src;
	uint32_t target_lpc;

	CONSUME_TMPVAR(src);
	CONSUME_IMM32(target_lpc);
	if (target_lpc >= ctx->func->bytecode_size + 1) {
		rt_error(ctx->rt, BROKEN_BYTECODE);
		return false;
	}

	/* Patch later. */
	ctx->branch_patch[ctx->branch_patch_count].code = ctx->code;
	ctx->branch_patch[ctx->branch_patch_count].lpc = target_lpc;
	ctx->branch_patch[ctx->branch_patch_count].type = PATCH_JE;
	ctx->branch_patch_count++;

	ASM {
		/* Patched later. */
		/* je 6 */				IB(0x0f); IB(0x84); ID(0);
	}

	return true;
}

/* Visit a bytecode of a function. */
bool
jit_visit_bytecode(
	struct jit_context *ctx)
{
	uint8_t opcode;

	/* Put a prologue. */
	ASM {
	/* prologue: */
		/* pushq %rax */			IB(0x50);
		/* pushq %rbx */			IB(0x53);
		/* pushq %rcx */			IB(0x51);
		/* pushq %rdx */			IB(0x52);
		/* pushq %rdi */			IB(0x57);
		/* pushq %rsi */			IB(0x56);
		/* pushq %r13 */			IB(0x41); IB(0x55);
		/* pushq %r14 */			IB(0x41); IB(0x56);
		/* pushq %r15 */			IB(0x41); IB(0x57);

		/* r14 = rt */
		/* movq %rdi, %r14 */			IB(0x49); IB(0x89); IB(0xfe);

		/* r15 = *&rt->frame->tmpvar[0] */
		/* movq (%r14), %rax */			IB(0x49); IB(0x8b); IB(0x06);
		/* movq (%rax), %r15 */			IB(0x4c); IB(0x8b); IB(0x38);

		/* r13 = exception_handler */
		/* movabs (ctx->code + 12), %r13 */	IB(0x49); IB(0xbd); IQ((intptr_t)(ctx->code + 10));

		/* Skip an exception handler. */
		/* jmp exception_handler_end */		IB(0xeb); IB(0x14);
	}

	/* Put an exception handler. */
	ctx->exception_code = ctx->code;
	ASM {
	/* exception_handler: */
		/* popq %r15 */ 	IB(0x41); IB(0x5f);
		/* popq %r14 */ 	IB(0x41); IB(0x5e);
		/* popq %r13 */ 	IB(0x41); IB(0x5d);
		/* popq %rsi */ 	IB(0x5e);
		/* popq %rdi */		IB(0x5f);
		/* popq %rdx */		IB(0x5a);
		/* popq %rcx */		IB(0x59);
		/* popq %rbx */		IB(0x5b);
		/* popq %rax */		IB(0x58);
		/* movq $0, %rax */	IB(0x48); IB(0xc7); IB(0xc0); ID(0);
		/* ret */		IB(0xc3);
	/* exception_handler_end: */
	}

	/* Put a body. */
	while (ctx->lpc < ctx->func->bytecode_size) {
		/* Save LPC and addr. */
		if (ctx->pc_entry_count >= PC_ENTRY_MAX) {
			rt_error(ctx->rt, "Too big code.");
			return false;
		}
		ctx->pc_entry[ctx->pc_entry_count].lpc = ctx->lpc;
		ctx->pc_entry[ctx->pc_entry_count].code = ctx->code;
		ctx->pc_entry_count++;

		/* Dispatch by opcode. */
		CONSUME_OPCODE(opcode);
		switch (opcode) {
		case ROP_LINEINFO:
			if (!jit_visit_lineinfo_op(ctx))
				return false;
			break;
		case ROP_ASSIGN:
			if (!jit_visit_assign_op(ctx))
				return false;
			break;
		case ROP_ICONST:
			if (!jit_visit_iconst_op(ctx))
				return false;
			break;
		case ROP_FCONST:
			if (!jit_visit_fconst_op(ctx))
				return false;
			break;
		case ROP_SCONST:
			if (!jit_visit_sconst_op(ctx))
				return false;
			break;
		case ROP_ACONST:
			if (!jit_visit_aconst_op(ctx))
				return false;
			break;
		case ROP_DCONST:
			if (!jit_visit_dconst_op(ctx))
				return false;
			break;
		case ROP_INC:
			if (!jit_visit_inc_op(ctx))
				return false;
			break;
		case ROP_ADD:
			if (!jit_visit_add_op(ctx))
				return false;
			break;
		case ROP_SUB:
			if (!jit_visit_sub_op(ctx))
				return false;
			break;
		case ROP_MUL:
			if (!jit_visit_mul_op(ctx))
				return false;
			break;
		case ROP_DIV:
			if (!jit_visit_div_op(ctx))
				return false;
			break;
		case ROP_MOD:
			if (!jit_visit_mod_op(ctx))
				return false;
			break;
		case ROP_AND:
			if (!jit_visit_and_op(ctx))
				return false;
			break;
		case ROP_OR:
			if (!jit_visit_or_op(ctx))
				return false;
			break;
		case ROP_XOR:
			if (!jit_visit_xor_op(ctx))
				return false;
			break;
		case ROP_NEG:
			if (!jit_visit_neg_op(ctx))
				return false;
			break;
		case ROP_LT:
			if (!jit_visit_lt_op(ctx))
				return false;
			break;
		case ROP_LTE:
			if (!jit_visit_lte_op(ctx))
				return false;
			break;
		case ROP_EQ:
			if (!jit_visit_eq_op(ctx))
				return false;
			break;
		case ROP_NEQ:
			if (!jit_visit_neq_op(ctx))
				return false;
			break;
		case ROP_GTE:
			if (!jit_visit_gte_op(ctx))
				return false;
			break;
		case ROP_GT:
			if (!jit_visit_gt_op(ctx))
				return false;
			break;
		case ROP_EQI:
			if (!jit_visit_eqi_op(ctx))
				return false;
			break;
		case ROP_LOADARRAY:
			if (!jit_visit_loadarray_op(ctx))
				return false;
			break;
		case ROP_STOREARRAY:
			if (!jit_visit_storearray_op(ctx))
				return false;
			break;
		case ROP_LEN:
			if (!jit_visit_len_op(ctx))
			return false;
			break;
		case ROP_GETDICTKEYBYINDEX:
			if (!jit_visit_getdictkeybyindex_op(ctx))
			return false;
			break;
		case ROP_GETDICTVALBYINDEX:
			if (!jit_visit_getdictvalbyindex_op(ctx))
				return false;
			break;
		case ROP_LOADSYMBOL:
			if (!jit_visit_loadsymbol_op(ctx))
				return false;
			break;
		case ROP_STORESYMBOL:
			if (!jit_visit_storesymbol_op(ctx))
				return false;
			break;
		case ROP_LOADDOT:
			if (!jit_visit_loaddot_op(ctx))
				return false;
			break;
		case ROP_STOREDOT:
			if (!jit_visit_storedot_op(ctx))
				return false;
			break;
		case ROP_CALL:
			if (!jit_visit_call_op(ctx))
				return false;
			break;
		case ROP_THISCALL:
			if (!jit_visit_thiscall_op(ctx))
				return false;
			break;
		case ROP_JMP:
			if (!jit_visit_jmp_op(ctx))
				return false;
			break;
		case ROP_JMPIFTRUE:
			if (!jit_visit_jmpiftrue_op(ctx))
				return false;
			break;
		case ROP_JMPIFFALSE:
			if (!jit_visit_jmpiffalse_op(ctx))
				return false;
			break;
		case ROP_JMPIFEQ:
			if (!jit_visit_jmpifeq_op(ctx))
				return false;
			break;
		default:
			assert(JIT_OP_NOT_IMPLEMENTED);
			break;
		}
	}

	/* Put an epilogue. */
	ASM {
	/* epilogue: */
		/* popq %r15 */ 	IB(0x41); IB(0x5f);
		/* popq %r14 */ 	IB(0x41); IB(0x5e);
		/* popq %r13 */ 	IB(0x41); IB(0x5d);
		/* popq %rsi */ 	IB(0x5e);
		/* popq %rdi */		IB(0x5f);
		/* popq %rdx */		IB(0x5a);
		/* popq %rcx */		IB(0x59);
		/* popq %rbx */		IB(0x5b);
		/* popq %rax */		IB(0x58);
		/* movq $1, %rax */	IB(0x48); IB(0xc7); IB(0xc0); ID(1);
		/* ret */		IB(0xc3);
	}

	return true;
}

static bool
jit_patch_branch(
    struct jit_context *ctx,
    int patch_index)
{
	uint8_t *target_code;
	int offset;
	int i;

	/* Search a code addr at lpc. */
	target_code = NULL;
	for (i = 0; i < ctx->pc_entry_count; i++) {
		if (ctx->pc_entry[i].lpc == ctx->branch_patch[patch_index].lpc) {
			target_code = ctx->pc_entry[i].code;
			break;
		}
			
	}
	if (target_code == NULL) {
		rt_error(ctx->rt, "Branch target not found.");
		return false;
	}

	/* Calc a branch offset. */
	offset = (intptr_t)target_code - (intptr_t)ctx->branch_patch[patch_index].code;

	/* Set the assembler cursor. */
	ctx->code = ctx->branch_patch[patch_index].code;

	/* Assemble. */
	if (ctx->branch_patch[patch_index].type == PATCH_JMP) {
		offset -= 5;
		ASM {
			/* jmp offset */
			IB(0xe9);
			ID(offset);
		}
	} else if (ctx->branch_patch[patch_index].type == PATCH_JE) {
		offset -= 6;
		ASM {
			/* je offset */
			IB(0x0f);
			IB(0x84);
			ID(offset);
		}
	} else if (ctx->branch_patch[patch_index].type == PATCH_JNE) {
		offset -= 6;
		ASM {
			/* jne offset */
			IB(0x0f);
			IB(0x85);
			ID(offset);
		}
	}

	return true;
}

#endif /* defined(ARCH_X86_64) && defined(USE_JIT) */
