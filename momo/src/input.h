/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Horizon
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * Interface of the "input" module.
 */

#ifndef HORIZON_HAL_INPUT_H
#define HORIZON_HAL_INPUT_H

enum input_button_code {
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_A,
	BUTTON_B,
	BUTTON_X,
	BUTTON_Y,
	BUTTON_L1,
	BUTTON_R1,
	BUTTON_L2,
	BUTTON_R2,
	BUTTON_CODE_SIZE,
};

enum input_key_code {
	KEY_RETURN,
	KEY_SPACE,
	KEY_CONTROL,
	KEY_DOWN,
	KEY_UP,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_CODE_SIZE,
};

/* Initialize the "input" module. */
bool input_init_module(void);

/* Cleanup the "input" module. */
void input_cleanup_module(void);

/* Check if a button is pressed. */
bool input_is_button_pressed(int button);

/* Get a pressure value of a button. */
float input_get_button_pressure(int button);

/* Check if a key is pressed. */
bool input_is_key_pressed(int key);

/* Get a stick X position. */
float input_get_stick_x(int stick);

/* Get a stick Y position. */
float input_get_stick_y(int stick);

/* Get a mouse X position. */
int input_get_mouse_x(void);

/* Get a mouse Y position. */
int input_get_mouse_y(void);

#endif
