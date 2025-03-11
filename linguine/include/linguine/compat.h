/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Horizon
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * A header to mitigate compiler differences.
 */

#ifndef HORIZON_COMPAT_H
#define HORIZON_COMPAT_H

/*
 * Here we define a macro that indicates a target platform.
 *
 * |Macro           |Platform              |
 * |----------------|----------------------|
 * |TARGET_WINDOWS  |Win64                 |
 * |TARGET_MACOS    |macS                  |
 * |TARGET_LINUX    |Linux (non-Android)   |
 * |TARGET_IOS      |iOS                   |
 * |TARGET_ANDROID  |Android NDK           |
 * |TARGET_WASM     |Wasm (Emscripten)     |
 */

/* Windows */
#if defined(_WIN32)
#define TARGET_WINDOWS
#endif

/* macOS */
#if defined(__APPLE__) && __has_include(<TargetConditionals.h>)
#include <TargetConditionals.h>
#if !defined(TARGET_OS_IPHONE)
#define TARGET_MACOS
#endif
#endif

/* Linux (non-Android) */
#if defined(__linux) && !defined(__ANDROID__)
#define TARGET_LINUX
#endif

/* iOS */
#if defined(__APPLE__) && __has_include(<TargetConditionals.h>)
#include <TargetConditionals.h>
#if defined(TARGET_OS_IPHONE)
#define TARGET_IOS
#endif
#endif

/* Android */
#if defined(__ANDROID__)
#define TARGET_ANDROID
#endif

/* Wasm (Emscripten) */
#if defined(__EMSCRIPTEN__)
#define TARGET_WASM
#endif

/* Error: No target detected. */
#if !defined(TARGET_WINDOWS) && \
    !defined(TARGET_MACOS) &&   \
    !defined(TARGET_LINUX) &&   \
    !defined(TARGET_IOS) &&     \
    !defined(TARGET_ANDROID) && \
    !defined(TARGET_WASM)
#error "No target detected."
#endif

/*
 * For GCC
 */
#if defined(__GNUC__) && !defined(__llvm__)

/* Define a macro that indicates a target architecture. */
#if defined(__i386__) && !defined(__x86_64__)
#define ARCH_X86
#elif defined(__x86_64__)
#define ARCH_X86_64
#elif defined(__arm__)
#define ARCH_ARM32
#elif defined(__aarch64__)
#define ARCH_ARM64
#endif

/* uint*_t and int*_t */
#include <stdint.h>

/* size_t */
#include <stddef.h>

/* bool */
#ifndef __cplusplus
#include <stdbool.h>
#endif

/* Inline function. */
#define INLINE				__inline

/* No pointer aliasing. */
#define RESTRICT			__restrict

/* Suppress unused warnings. */
#define UNUSED_PARAMETER(x)		(void)(x)

/* UTF-8 string literal. */
#define U8(s)				u8##s

/* UTF-32 character literal. */
#define U32_C(literal, unicode)		U##literal

/* Parameter modifiers. */
#define IN
#define OUT
#define INOUT

#endif /* End of GCC */

/*
 * For Clang/LLVM
 */
#if defined(__llvm__)

#if defined(__i386__) && !defined(__x86_64__)
#define ARCH_X86
#elif defined(__x86_64__)
#define ARCH_X86_64
#elif defined(__arm__)
#define ARCH_ARM32
#elif defined(__aarch64__)
#define ARCH_ARM64
#endif

#include <stdint.h>
#include <stddef.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

#define INLINE				__inline
#define RESTRICT			__restrict
#define UNUSED_PARAMETER(x)		(void)(x)
#define U8(s)				u8##s
#define U32_C(literal, unicode)		U##literal

#define IN
#define OUT
#define INOUT

#endif /* End of Clang/LLVM */

/*
 * For MSVC
 */
#ifdef _MSC_VER

#if defined(_M_IX86)
#define ARCH_X86
#elif defined(_M_X64)
#define ARCH_X86_64
#elif defined(_M_ARM64)
#define OPENNOVEL_ARCH_ARM64
#endif

/* Do not get warnings for usages of string.h functions. */
#define _CRT_SECURE_NO_WARNINGS

/* POSIX libc to MSVCRT mapping */
#define strdup _strdup
#define strcasecmp _stricmp

#include <stdint.h>
#include <stddef.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

#define INLINE				__inline
#define RESTRICT			__restrict
#define UNUSED_PARAMETER(x)		(void)(x)
#define U8(s)				u8##s
#define U32_C(literal, unicode)		U##literal

#define IN
#define OUT
#define INOUT

#endif /* End of MSVC */

/*
 * Byteorder
 */

#define HOSTTOLE64(d)	(d)
#define HOSTTOLE32(d)	(d)
#define HOSTTOLE16(d)	(d)

#define LETOHOST64(d)	(d)
#define LETOHOST32(d)	(d)
#define LETOHOST16(d)	(d)

/*
 * Message Translation
 */

#define _(text)		text

#endif
