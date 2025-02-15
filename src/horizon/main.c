#include "config.h"
#include "linguine/runtime.h"

#include <stdio.h>
#include <signal.h>

/* Scripting runtime. */
struct rt_env *rt;

/* Is Control-C pressed?*/
volatile bool is_stopped;

/* Temporary file content storage. */
char file_text[65536];

/* Forward declaration. */
void sigint_handler(int signal);
bool load_engine_object(struct rt_env *rt);
bool load_main_script(struct rt_env *rt);
bool load_file(const char *file_name);
bool call_init(struct rt_env *rt);
bool call_frame(struct rt_env *rt);
void print_error(struct rt_env *rt);
bool engine_print(struct rt_env *rt);

int main(int argc, char *argv[])
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

	/* Call OnInit(). */
	if (!call_init(rt))
		return false;

	/* Call OnFrame() until Control-C is pressed. */
//	while (!is_stopped) {
		if (!call_frame(rt))
			return false;
//	}

	/* Cleanup the scripting runtime. */
	rt_destroy(rt);
}

void sigint_handler(int signal)
{
	is_stopped = true;
}

bool load_engine_object(struct rt_env *rt)
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
		{"Engine_print", "print", 2, {"this", "msg"}, engine_print},
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

bool load_main_script(struct rt_env *rt)
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

bool load_file(const char *file_name)
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

bool call_init(struct rt_env *rt)
{
	struct rt_value ret;

	if (!rt_call_with_name(rt, "onInit", 0, NULL, &ret)) {
		print_error(rt);
		return false;;
	}

	rt_shallow_gc(rt);

	return true;
}

bool call_frame(struct rt_env *rt)
{
	struct rt_value ret;

	if (!rt_call_with_name(rt, "onFrame", 0, NULL, &ret)) {
		print_error(rt);
		return false;
	}

	rt_shallow_gc(rt);

	return true;
}

void print_error(struct rt_env *rt)
{
	printf("%s:%d: error: %s\n",
	       rt_get_error_file(rt),
	       rt_get_error_line(rt),
	       rt_get_error_message(rt));
}

bool engine_print(struct rt_env *rt)
{
	struct rt_value msg;
	const char *s;
	float f;
	int i;
	int type;

	if (!rt_get_local(rt, "msg", &msg))
		return false;

	if (!rt_get_value_type(rt, &msg, &type))
		return false;

	switch (type) {
	case RT_VALUE_INT:
		if (!rt_get_int(rt, &msg, &i))
			return false;
		printf("%i\n", i);
		break;
	case RT_VALUE_FLOAT:
		if (!rt_get_float(rt, &msg, &f))
			return false;
		printf("%f\n", f);
		break;
	case RT_VALUE_STRING:
		if (!rt_get_string(rt, &msg, &s))
			return false;
		printf("%s\n", s);
		break;
	default:
		printf("[object]\n");
		break;
	}

	return true;
}
