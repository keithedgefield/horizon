#include "config.h"
#include "linguine/runtime.h"

#include <stdio.h>
#include <signal.h>

/* Scripting runtime. */
struct rt_env *rt;

/* Renderer name. */
struct rt_func *renderer;

/* Is ctrl-c pressed? */
volatile bool is_stopped;

/* Temporary file content storage. */
char file_text[65536];

/* Forward declaration. */
static void sigint_handler(int signal);
static bool load_engine_object(struct rt_env *rt);
static bool load_main_script(struct rt_env *rt);
static bool load_file(const char *file_name);
static bool call_main(struct rt_env *rt);
static bool get_renderer(struct rt_env *rt);
static bool call_renderer(struct rt_env *rt);
static void print_error(struct rt_env *rt);
static bool engine_print(struct rt_env *rt);

int init_engine(int argc, char *argv[])
{
	struct rt_value ret;

	signal(SIGINT, sigint_handler);

	/* Create a scripting runtime. */
	if (!rt_create(&rt)) {
		printf("Error.\n");
		return 1;
	}

	/* Load "Engine" object. */
	if (!load_engine_object(rt))
		return false;

	/* Load the main script. */
	if (!load_main_script(rt))
		return false;

	/* Call main(). */
	if (!call_main(rt))
		return false;

	/* Get a renderer function name. */
	if (!get_renderer(rt))
		return false;

	/* Call OnFrame() until Control-C is pressed. */
	while (is_stopped) {
		if (!call_renderer(rt))
			return false;
	}

	/* Cleanup the scripting runtime. */
	rt_destroy(rt);
}

static void sigint_handler(int sig)
{
	is_stopped = true;
}

static bool load_engine_object(struct rt_env *rt)
{
	struct rt_value engine;
	int i;
	struct item {
		const char *global_name;
		const char *key_name;
		int param_count;
		const char *param_name[16];
		bool (*cfunc)(struct rt_env *env);
	} item[] = {
		{"Engine_print", "print", 1, {"msg"}, engine_print},
	};

	/* Make a dictionary. */
	if (!rt_make_empty_dict(rt, &engine))
		return false;

	/* Assign the dictionary to a global variable "Engine". */
	if (!rt_set_global(rt, "Engine", &engine))
		return false;

	/* For each entry, register a C function,. */
	for (i = 0; i < sizeof(item) / sizeof(struct item); i++) {
		struct rt_value func_val;

		/* Register a C function as a global variable. */
		if (!rt_register_cfunc(rt, item[i].global_name, item[i].param_count, item[i].param_name, item[i].cfunc))
			return false;

		/* Put the function to the dictionary. */
		if (!rt_get_global(rt, item[i].global_name, &func_val))
			return false;
		if (!rt_set_dict_elem(rt, &engine, item[i].key_name, &func_val))
			return false;
	}

	return true;
}

static bool load_main_script(struct rt_env *rt)
{
	if (!load_file("main.ls"))
		return false;

	/* Compile. */
	if (!rt_register_source(rt, "main.ls", file_text)) {
		print_error(rt);
		return false;
	}

	return true;
}

static bool load_file(const char *file_name)
{
	FILE *fp;
	size_t len;

	fp = fopen(file_name, "rb");
	if (fp == NULL) {
		printf("Cannot open file.\n");
		return false;
	}

	len = fread(file_text, 1, sizeof(file_text) - 1, fp);
	if (len == 0) {
		printf("Cannot read the file.\n");
		return false;
	}

	/* Terminate the string. */
	file_text[len] = '\0';

	fclose(fp);

	return true;
}

static bool call_main(struct rt_env *rt)
{
	struct rt_value ret;

	if (!rt_call_with_name(rt, "main", NULL, 0, NULL, &ret)) {
		print_error(rt);
		return false;
	}

	rt_shallow_gc(rt);

	return true;
}

static bool get_renderer(struct rt_env *rt)
{
	struct rt_value dict, elem;

	if (!rt_get_global(rt, "Engine", &dict))
		return false;

	if (!rt_get_dict_elem(rt, &dict, "renderer", &elem)) {
		printf("Engine.renderer not defined.\n");
		return false;
	}

	if (!rt_get_func(rt, &elem, &renderer))
		return false;

	return true;
}

static bool call_renderer(struct rt_env *rt)
{
	struct rt_value ret;

	if (!rt_call(rt, renderer, NULL, 0, NULL, &ret)) {
		print_error(rt);
		return false;
	}

	rt_shallow_gc(rt);

	return true;
}

static void print_error(struct rt_env *rt)
{
	printf("%s:%d: error: %s\n",
	       rt_get_error_file(rt),
	       rt_get_error_line(rt),
	       rt_get_error_message(rt));
}
