/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * GameKit
 * Copyright (c) 2025, The Horizon Authors. All rights reserved.
 */

/*
 * linuxmain.c: Linux main()
 */

#include "gamekit/gamekit.h"

#include "stdfile.h"
#include "stdimage.h"
#include "glrender.h"

/* X11 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/Xatom.h>
#include <X11/Xlocale.h>

/* OpenGL */
#include <GL/glx.h>

/* POSIX */
#include <sys/types.h>
#include <sys/stat.h>	/* stat(), mkdir() */
#include <sys/time.h>	/* gettimeofday() */
#include <unistd.h>	/* usleep(), access() */

/*
 * Framerate
 */

/* Frame Time */
#define FRAME_MILLI	(16)	/* Millisec of a frame */
#define SLEEP_MILLI	(5)	/* Millisec to sleep */

/* Frame start time */
static struct timeval tv_start;

/*
 * Window
 */

/* Window title */
static char *window_title;

/* Window size */
static int window_width;
static int window_height;

/* X11 Objects */
Display *display;
static Window window = None;
static GLXWindow glx_window = None;
static GLXContext glx_context = None;
static Atom delete_message = BadAlloc;
static Pixmap icon = BadAlloc;
static Pixmap icon_mask = BadAlloc;

/*
 * OpenGL
 */

/* OpenGL 2+ symbols */
GLuint (APIENTRY *glCreateShader)(GLenum type);
void (APIENTRY *glShaderSource)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
void (APIENTRY *glCompileShader)(GLuint shader);
void (APIENTRY *glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
void (APIENTRY *glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
void (APIENTRY *glAttachShader)(GLuint program, GLuint shader);
void (APIENTRY *glLinkProgram)(GLuint program);
void (APIENTRY *glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
void (APIENTRY *glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GLuint (APIENTRY *glCreateProgram)(void);
void (APIENTRY *glUseProgram)(GLuint program);
void (APIENTRY *glGenVertexArrays)(GLsizei n, GLuint *arrays);
void (APIENTRY *glBindVertexArray)(GLuint array);
void (APIENTRY *glGenBuffers)(GLsizei n, GLuint *buffers);
void (APIENTRY *glBindBuffer)(GLenum target, GLuint buffer);
GLint (APIENTRY *glGetAttribLocation)(GLuint program, const GLchar *name);
void (APIENTRY *glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
void (APIENTRY *glEnableVertexAttribArray)(GLuint index);
GLint (APIENTRY *glGetUniformLocation)(GLuint program, const GLchar *name);
void (APIENTRY *glUniform1i)(GLint location, GLint v);
void (APIENTRY *glUniform1f)(GLint location, GLfloat v);
void (APIENTRY *glUniform2fv)(GLint location, GLfloat *v);
void (APIENTRY *glUniform3fv)(GLint location, GLfloat *v);
void (APIENTRY *glUniform4fv)(GLint location, GLfloat *v);
void (APIENTRY *glUniformMatrix4fv)(GLint location, GLfloat *v);
void (APIENTRY *glBufferData)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
void (APIENTRY *glDeleteShader)(GLuint shader);
void (APIENTRY *glDeleteProgram)(GLuint program);
void (APIENTRY *glDeleteVertexArrays)(GLsizei n, const GLuint *arrays);
void (APIENTRY *glDeleteBuffers)(GLsizei n, const GLuint *buffers);

/* Symbol table */
struct API {
	void **func;
	const char *name;
};
static struct API api[] = {
	{(void **)&glCreateShader, "glCreateShader"},
	{(void **)&glShaderSource, "glShaderSource"},
	{(void **)&glCompileShader, "glCompileShader"},
	{(void **)&glGetShaderiv, "glGetShaderiv"},
	{(void **)&glGetShaderInfoLog, "glGetShaderInfoLog"},
	{(void **)&glAttachShader, "glAttachShader"},
	{(void **)&glLinkProgram, "glLinkProgram"},
	{(void **)&glGetProgramiv, "glGetProgramiv"},
	{(void **)&glGetProgramInfoLog, "glGetProgramInfoLog"},
	{(void **)&glCreateProgram, "glCreateProgram"},
	{(void **)&glUseProgram, "glUseProgram"},
	{(void **)&glGenVertexArrays, "glGenVertexArrays"},
	{(void **)&glBindVertexArray, "glBindVertexArray"},
	{(void **)&glGenBuffers, "glGenBuffers"},
	{(void **)&glBindBuffer, "glBindBuffer"},
	{(void **)&glGetAttribLocation, "glGetAttribLocation"},
	{(void **)&glVertexAttribPointer, "glVertexAttribPointer"},
	{(void **)&glEnableVertexAttribArray, "glEnableVertexAttribArray"},
	{(void **)&glGetUniformLocation, "glGetUniformLocation"},
	{(void **)&glUniform1i, "glUniform1i"},
	{(void **)&glUniform1f, "glUniform1f"},
	{(void **)&glUniform2fv, "glUniform2fv"},
	{(void **)&glUniform3fv, "glUniform3fv"},
	{(void **)&glUniform4fv, "glUniform4fv"},
	{(void **)&glUniformMatrix4fv, "glUniformMatrix4fv"},
	{(void **)&glBufferData, "glBufferData"},
	{(void **)&glDeleteShader, "glDeleteShader"},
	{(void **)&glDeleteProgram, "glDeleteProgram"},
	{(void **)&glDeleteVertexArrays, "glDeleteVertexArrays"},
	{(void **)&glDeleteBuffers, "glDeleteBuffers"},
};

/*
 * Input
 */

/* Key state */
static bool is_key_pressed[KEY_CODE_SIZE];

/* Mouse button state */
static bool is_mouse_left_pressed;
static bool is_mouse_right_pressed;
static bool is_mouse_up_pressed;
static bool is_mouse_down_pressed;

/* Mouse position */
static int mouse_pos_x;
static int mouse_pos_y;

/*
 * Logging
 */

#define LOG_BUF_SIZE	(4096)

/*
 * Forward declaration
 */

static bool init_window(void);
static void cleanup_window(void);
static void run_game_loop(void);
static bool wait_for_next_frame(void);
static bool next_event(void);
static void on_key_press(XEvent *event);
static void on_key_release(XEvent *event);
static int get_key_code(XEvent *event);
static void on_button_press(XEvent *event);
static void on_button_release(XEvent *event);
static void on_motion_notify(XEvent *event);
static char *make_path(const char *path);

/*
 * Main
 */
int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");

	/* Initialize the stdfile module. */
	if (!stdfile_init(make_path))
		return 1;

	/* Tell application that HAL is going to initialize the "render" module. */
	if (!on_hal_init_render(&window_title, &window_width, &window_height))
		return 1;

	/* Initializa the stdimage module. */
	if (!stdimage_init())
		return 1;

	/* Initialize the window. */
	if (!init_window())
		return 1;

	/* Tell application that HAL is ready. */
	if (!on_hal_ready())
		return 1;

	/* Run the game loop */
	run_game_loop();

	/* Cleanup the window. */
	cleanup_window();

	/* Cleanup the stdimage module. */
	stdfile_cleanup();

	/* Cleanup the stdfile module. */
	stdfile_cleanup();

	return 0;
}

bool init_window(void)
{
	int pix_attr[] = {
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DOUBLEBUFFER, True,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		None
	};
	int ctx_attr[]= {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 2,
		GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		GLX_CONTEXT_FLAGS_ARB, 0,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};
	GLXContext (*glXCreateContextAttribsARB)(Display *dpy,
						 GLXFBConfig config,
						 GLXContext share_context,
						 Bool direct,
						 const int *attrib_list);
	GLXFBConfig *config;
	XVisualInfo *vi;
	XSetWindowAttributes swa;
	XSizeHints *sh;
	XTextProperty tp;
	XEvent event;
	int i, n, ret;

	/* Open a display. */
	display = XOpenDisplay(NULL);
	if (display == NULL) {
		printf("Cannot open display.\n");
		return false;
	}

	/* Choose a framebuffer format. */
	config = glXChooseFBConfig(display, DefaultScreen(display), pix_attr, &n);
	if (config == NULL)
		return false;
	vi = glXGetVisualFromFBConfig(display, config[0]);

	/* Create a window. */
	swa.border_pixel = 0;
	swa.event_mask = StructureNotifyMask;
	swa.colormap = XCreateColormap(display,
				       RootWindow(display, vi->screen),
				       vi->visual,
				       AllocNone);
	window = XCreateWindow(display,
			       RootWindow(display, vi->screen),
			       0,
			       0,
			       (unsigned int)window_width,
			       (unsigned int)window_height,
			       0,
			       vi->depth,
			       InputOutput,
			       vi->visual,
			       CWBorderPixel | CWColormap | CWEventMask,
			       &swa);
	XFree(vi);

	/* Create a GLX context. */
	glXCreateContextAttribsARB = (void *)glXGetProcAddress((const unsigned char *)"glXCreateContextAttribsARB");
	if (glXCreateContextAttribsARB == NULL) {
		sys_error("glXGetProcAddress() for glXCreateContextAttribsARB failed.");
		XDestroyWindow(display, window);
		return false;
	}
	glx_context = glXCreateContextAttribsARB(display, config[0], 0, True, ctx_attr);
	if (glx_context == NULL) {
		sys_error("glXCreateContextArrtibsARB() failed.");
		XDestroyWindow(display, window);
		return false;
	}

	/* Create a GLX window. */
	glx_window = glXCreateWindow(display, config[0], window, NULL);
	XFree(config);

	/* Map the window to the screen, and wait for showing. */
	XMapWindow(display, window);
	XNextEvent(display, &event);

	/* Bind the GLX context to the window. */
	glXMakeContextCurrent(display, glx_window, glx_window, glx_context);

	/* Get the API pointers. */
	for (i = 0; i < (int)(sizeof(api)/sizeof(struct API)); i++) {
		*api[i].func = (void *)glXGetProcAddress((const unsigned char *)api[i].name);
		if(*api[i].func == NULL) {
			sys_error("Failed to get API %s().", api[i].name);
			glXMakeContextCurrent(display, None, None, None);
			glXDestroyContext(display, glx_context);
			glXDestroyWindow(display, glx_window);
			glx_context = None;
			glx_window = None;
			return false;
		}
	}

	/* Initialize the OpenGL rendering subsystem. */
	if (!glrender_init(0, 0, window_width, window_height)) {
		glXMakeContextCurrent(display, None, None, None);
		glXDestroyContext(display, glx_context);
		glXDestroyWindow(display, glx_window);
		glx_context = None;
		glx_window = None;
		return false;
	}

	/* Set the window title. */
	ret = XmbTextListToTextProperty(display, &window_title, 1, XCompoundTextStyle, &tp);
	if (ret == XNoMemory || ret == XLocaleNotSupported) {
		sys_error("XmbTextListToTextProperty() failed.");
		return false;
	}
	XSetWMName(display, window, &tp);
	XFree(tp.value);

	/* Show the window. */
	ret = XMapWindow(display, window);
	if (ret == BadWindow) {
		sys_error("XMapWindow() failed.");
		return false;
	}

	/* Set the fixed window size. */
	sh = XAllocSizeHints();
	sh->flags = PMinSize | PMaxSize;
	sh->min_width = window_width;
	sh->min_height = window_height;
	sh->max_width = window_width;
	sh->max_height = window_height;
	XSetWMSizeHints(display, window, sh, XA_WM_NORMAL_HINTS);
	XFree(sh);

	/* Set the event mask. */
	ret = XSelectInput(display,
			   window,
			   KeyPressMask |
			   ExposureMask |
			   ButtonPressMask |
			   ButtonReleaseMask |
			   KeyReleaseMask |
			   PointerMotionMask);
	if (ret == BadWindow) {
		sys_error("XSelectInput() failed.");
		return false;
	}

	/* Capture close button events if possible. */
	delete_message = XInternAtom(display, "WM_DELETE_WINDOW", True);
	if (delete_message != None && delete_message != BadAlloc && delete_message != BadValue)
		XSetWMProtocols(display, window, &delete_message, 1);

	return true;
}

void cleanup_window(void)
{
	if (display != NULL)
		glXMakeContextCurrent(display, None, None, None);

	if (glx_context != None) {
		glXDestroyContext(display, glx_context);
		glx_context = None;
	}

	if (glx_window != None) {
		glXDestroyWindow(display, glx_window);
		glx_window = None;
	}

	if (window != None) {
		XDestroyWindow(display, window);
		window = None;
	}

	if (display != None) {
		XCloseDisplay(display);
		display = None;
	}
}

/* Run a game loop. */
static void run_game_loop(void)
{
	/* Get the frame start time. */
	gettimeofday(&tv_start, NULL);

	/* Main Loop. */
	while (true) {
		/* Run a frame. */
		if (!on_hal_frame())
			break;

		/* Swap buffers. */
		glXSwapBuffers(display, glx_window);

		/* Wait for the next frame timing. */
		if (!wait_for_next_frame())
			break;	/* Close button was pressed. */

		/* Get the frame start time. */
		gettimeofday(&tv_start, NULL);
	}
}

/* Wait for the next frame timing. */
static bool wait_for_next_frame(void)
{
	struct timeval tv_end;
	uint32_t lap, wait, span;

	span = FRAME_MILLI;

	/* Do event processing and sleep until the time of next frame start. */
	do {
		/* Process events if exist. */
		while (XEventsQueued(display, QueuedAfterFlush) > 0)
			if (!next_event())
				return false;

		/* Get a lap time. */
		gettimeofday(&tv_end, NULL);
		lap = (uint32_t)((tv_end.tv_sec - tv_start.tv_sec) * 1000 +
				 (tv_end.tv_usec - tv_start.tv_usec) / 1000);

		/* Break if the time has come. */
		if (lap > span) {
			tv_start = tv_end;
			break;
		}

		/* Calc a sleep time. */
		wait = (span - lap > SLEEP_MILLI) ? SLEEP_MILLI : span - lap;

		/* Do sleep. */
		usleep(wait * 1000);

	} while(wait > 0);

	return true;
}

/* Process an event. */
static bool next_event(void)
{
	XEvent event;

	XNextEvent(display, &event);
	switch (event.type) {
	case KeyPress:
		on_key_press(&event);
		break;
	case KeyRelease:
		on_key_release(&event);
		break;
	case ButtonPress:
		on_button_press(&event);
		break;
	case ButtonRelease:
		on_button_release(&event);
		break;
	case MotionNotify:
		on_motion_notify(&event);
		break;
	case MappingNotify:
		XRefreshKeyboardMapping(&event.xmapping);
		break;
	case ClientMessage:
		/* Close button was pressed. */
		if ((Atom)event.xclient.data.l[0] == delete_message)
			return false;
		break;
	}
	return true;
}

/* Process a KeyPress event. */
static void on_key_press(XEvent *event)
{
	int key;

	/* Get a key code. */
	key = get_key_code(event);
	if (key == -1)
		return;
	assert(key < KEY_CODE_SIZE);

	is_key_pressed[key] = true;
}

/* Process a KeyRelease event. */
static void on_key_release(XEvent *event)
{
	int key;

	/* Ignore auto repeat events. */
	if (XEventsQueued(display, QueuedAfterReading) > 0) {
		XEvent next;
		XPeekEvent(display, &next);
		if (next.type == KeyPress &&
		    next.xkey.keycode == event->xkey.keycode &&
		    next.xkey.time == event->xkey.time) {
			XNextEvent(display, &next);
			return;
		}
	}

	/* Get a key code. */
	key = get_key_code(event);
	if (key == -1)
		return;
	assert(key < KEY_CODE_SIZE);

	is_key_pressed[key] = false;
}

/* Convert 'KeySym' to 'enum key_code'. */
static int get_key_code(XEvent *event)
{
	char text[255];
	KeySym keysym;

	/* Get a KeySym. */
	XLookupString(&event->xkey, text, sizeof(text), &keysym, 0);

	/* Convert to key_code. */
	switch (keysym) {
	case XK_Return:
	case XK_KP_Enter:
		return KEY_RETURN;
	case XK_space:
		return KEY_SPACE;
	case XK_Control_L:
	case XK_Control_R:
		return KEY_CONTROL;
	case XK_Down:
		return KEY_DOWN;
	case XK_Up:
		return KEY_UP;
	case XK_Left:
		return KEY_LEFT;
	case XK_Right:
		return KEY_RIGHT;
	default:
		break;
	}
	return -1;
}

/* Process a ButtonPress event. */
static void on_button_press(XEvent *event)
{
	switch (event->xbutton.button) {
	case Button1:
		is_mouse_left_pressed = true;
		break;
	case Button3:
		is_mouse_right_pressed = true;
		break;
	case Button4:
		is_mouse_up_pressed = true;
		break;
	case Button5:
		is_mouse_down_pressed = true;
		break;
	default:
		break;
	}
}

/* Process a ButtonPress event. */
static void on_button_release(XEvent *event)
{
	switch (event->xbutton.button) {
	case Button1:
		is_mouse_left_pressed = true;
		break;
	case Button3:
		is_mouse_right_pressed = true;
		break;
	}
}

/* Process a MotionNotify event. */
static void on_motion_notify(XEvent *event)
{
	mouse_pos_x = event->xmotion.x;
	mouse_pos_y = event->xmotion.y;
}


/*
 * "input" interface implementation
 */

/*
 * Check if a button is pressed.
 */
bool input_is_button_pressed(int button)
{
	return false;
}

/*
 * Get a pressure value of a button.
 */
float input_get_button_pressure(int button)
{
	return 0.0f;
}

/*
 * Check if a key is pressed.
 */
bool input_is_key_pressed(int key)
{
	assert(key < KEY_CODE_SIZE);

	if (is_key_pressed[key])
		return true;

	return false;
}

/*
 * Get a stick X position.
 */
float input_get_stick_x(int stick)
{
	return 0.0f;
}

/*
 * Get a analog stick Y position.
 */
float input_get_stick_y(int stick)
{
	return 0.0f;
}

/*
 * Get a mouse X position.
 */
int input_get_mouse_x(void)
{
	return mouse_pos_x;
}

/*
 * Get a mouse Y position.
 */
int input_get_mouse_y(void)
{
	return mouse_pos_y;
}


/*
 * sys_ interface implementation
 */

/*
 * Print a log line.
 */
void sys_log(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
}

/*
 * Print an error line.
 */
void sys_error(const char *s, ...)
{
	char buf[LOG_BUF_SIZE];
	va_list ap;

	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
}

/*
 * Print an out-of-memory log.
 */
void sys_out_of_memory(void)
{
	printf("Out of memory.");
}

/*
 * Get a millisecond time.
 */
uint64_t sys_get_tick(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/*
 * Get a two-letter language code of a system.
 */
const char *sys_get_language(void)
{
	const char *locale;

	locale = setlocale(LC_ALL, NULL);
	if (locale == NULL)
		return "en";

	if (locale[0] == '\0' || locale[1] == '\0')
		return "en";
	else if (strncmp(locale, "en", 2) == 0)
		return "en";
	else if (strncmp(locale, "fr", 2) == 0)
		return "fr";
	else if (strncmp(locale, "de", 2) == 0)
		return "de";
	else if (strncmp(locale, "it", 2) == 0)
		return "it";
	else if (strncmp(locale, "es", 2) == 0)
		return "es";
	else if (strncmp(locale, "el", 2) == 0)
		return "el";
	else if (strncmp(locale, "ru", 2) == 0)
		return "ru";
	else if (strncmp(locale, "zh_CN", 5) == 0)
		return "zh";
	else if (strncmp(locale, "zh_TW", 5) == 0)
		return "tw";
	else if (strncmp(locale, "ja", 2) == 0)
		return "ja";

	return "en";
}

/*
 * For the stdfile module.
 */
static char *make_path(const char *path)
{
	char *s;
	s = strdup(path);
	if (s == NULL) {
		sys_out_of_memory();
		return NULL;
	}
	return s;
}
