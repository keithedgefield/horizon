/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * MediaKit API Definition
 */

#ifndef MEDIAKIT_API_H
#define MEDIAKIT_API_H

#include "config.h"

typedef bool MKbool;
typedef int32_t MKint;
typedef uint32_t MKuint;
typedef uint32_t MKsize;
typedef float MKfloat;

/*
 * Common
 */
MKbool mkGetErrorMessage(MKuint *size, const char **message);

/*
 * Graphics
 */

enum MKShaderInputType {
	MK_SHADER_INPUT_TYPE_POSITION,
	MK_SHADER_INPUT_TYPE_TEXCOORD0,
	MK_SHADER_INPUT_TYPE_TEXCOORD1,
	MK_SHADER_INPUT_TYPE_COLOR0,
	MK_SHADER_INPUT_TYPE_COLOR1,
};

/* Rendering */
MKbool mkBeginScene(void);
MKbool mkBindShaderPipeline(MKuint pipeline);
MKbool mkBindVertexBuffer(MKuint buf);
MKbool mkBindIndexBuffer(MKuint buf);
MKbool mkBindConstantBuffer(MKuint slot, MKuint buf);
MKbool mkBindTexture(MKuint slot, MKuint tex);
MKbool mkDrawTriangles(MKuint list, MKuint count, MKuint offset);
MKbool mkDrawTriangleStrip(MKuint list, MKuint count, MKuint offset);
MKbool mkEndScene(void);

/* Vertex Buffer */
MKbool mkCreateVertexBuffer(MKuint size, MKuint *buf);
MKbool mkUpdateVertexBuffer(MKuint buf, MKfloat *data, MKuint offset, MKuint size);
MKbool mkDeleteVertexBuffer(MKuint buf);

/* Index Buffer */
MKbool mkCreateIndexBuffer(MKuint size, MKuint *buf);
MKbool mkUpdateIndexBuffer(MKuint buf, MKfloat *data, MKuint offset, MKuint size);
MKbool mkDeleteIndexBuffer(MKuint buf);

/* Constant Buffer */
MKbool mkCreateConstantBuffer(MKuint size, MKuint *buf);
MKbool mkUpdateConstantBuffer(MKuint buf, MKfloat *data, MKuint offset, MKuint size);
MKbool mkDeleteConstantBuffer(MKuint buf);

/* Shader Pipeline */
MKbool mkBeginShaderPipeline(MKuint *pipeline);
MKbook mkDefineShaderTexture(void);
MKbook mkDefineShaderSampler(void);
MKbool mkDefineShaderConstantFloatN(int slot, int elem);
MKbool mkDefineVertexShaderInputFloatN(MKuint elem, MKShaderInputType type);
MKbool mkDefineFragmentShaderInputFloatN(MKuint elem, MKShaderInputType type);
MKbool mkAssignVertexShaderIn(const char *dst, int n);
MKbool mkAssignVertexShaderConst(const char *dst, int slot, int index);
MKbool mkAssignVertexShaderLet(const char *dst, const char *rhs);
MKbool mkAssignVertexShaderOut(int n, const char *src);
MKbool mkAssignFragmentShaderIn(const char *dst, int n);
MKbool mkAssignFragmentShaderTex(const char *dst, int tex, int sampler);
MKbool mkAssignFragmentShaderLet(const char *dst, const char *rhs);
MKbool mkAssignFragmentShaderOut(const char *rhs);
MKbool mkEndShaderPipieline(void);
MKbool mkDeleteShaderPipeline(MKuint pipeline);

/* Video */
MKbool mkPlayVideo(const char *file);
MKbool mkIsVideoPlaying(void);
MKbool mkStopVideo(void);

/*
 * Audio
 */

typedef (*MKAudioCallback)(void *p, MKuint samples, MKuint *data);

MKbool mkCreateAudioStream(MKAudioCallback callback, void *p, MKuint *stream);
MKbool mkStartAudioStream(MKuint stream);
MKbool mkStopAudioStream(MKuint stream);
MKbool mkSetAudioStreamVolume(MKuint stream, MKfloat vol);
MKbool mkDeleteAudioStream(MKuint stream);

/*
 * Input
 */

enum MKJoyButton {
	/* Arrow1 */
	MK_JOY_BUTTON_ARROW1_UP,
	MK_JOY_BUTTON_ARROW1_DOWN,
	MK_JOY_BUTTON_ARROW1_RIGHT,
	MK_JOY_BUTTON_ARROW1_LEFT,

	/* Arrow2 */
	MK_JOY_BUTTON_ARROW2_UP,
	MK_JOY_BUTTON_ARROW2_DOWN,
	MK_JOY_BUTTON_ARROW2_RIGHT,
	MK_JOY_BUTTON_ARROW2_LEFT,

	/* Decide */
	MK_JOY_BUTTON_A,
	MK_JOY_BUTTON_B,
	MK_JOY_BUTTON_C,
	MK_JOY_BUTTON_D,
	MK_JOY_BUTTON_S1,	/* Start */
	MK_JOY_BUTTON_S2,	/* Select */

	/* L/R */
	MK_JOY_BUTTON_L1,
	MK_JOY_BUTTON_L2,
	MK_JOY_BUTTON_R1,
	MK_JOY_BUTTON_R2,
};

enum MKKeyCode {
	MK_A,
	MK_B,
	MK_C,
	MK_D,
	MK_E,
	MK_F,
	MK_G,
	MK_H,
	MK_I,
	MK_J,
	MK_K,
	MK_L,
	MK_M,
	MK_N,
	MK_O,
	MK_P,
	MK_Q,
	MK_R,
	MK_S,
	MK_T,
	MK_U,
	MK_V,
	MK_W,
	MK_X,
	MK_Y,
	MK_Z,
	MK_ARROW_UP,
	MK_ARROW_DOWN,
	MK_ARROW_RIGHT,
	MK_ARROW_LEFT,
	MK_HOME,
	MK_END,
	MK_PAGEUP,
	MK_PAGEDOWN,
	MK_INSERT,
	MK_DELETE,
	MK_F1,
	MK_F2,
	MK_F3,
	MK_F4,
	MK_F5,
	MK_F6,
	MK_F7,
	MK_F8,
	MK_F9,
	MK_F10,
	MK_F11,
	MK_F12,
	MK_ESC,
	MK_ENTER,
	MK_BACKSPACE,
	MK_TAB,
	MK_CONTROL,
	MK_SHIFT,
	MK_ALT,
	MK_COMMAND,
};

MKbool mkGetJoyCount(MKuint *count);
MKbool mkGetJoyDigitalState(MKuint n, MKJoyButton button, MKbool *state);
MKbool mkGetJoyAnalogState(MKuint n, MKJoyButton button, MKfloat *state);
MKbool mkGetKeyState(MKKeyCode key, MKbool *state);

#endif
