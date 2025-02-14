#include "ast.h"
#include "hir.h"
#include "lir.h"
#include "runtime.h"

#include <stdio.h>

char text[65536];

bool load_file(int argc, char *argv[]);
bool ffi_print(struct rt_env *rt);

int main(int argc, char *argv[])
{
	struct rt_env *rt;
	struct rt_value ret;
	const char *params[] = {"msg"};

	if (!load_file(argc, argv))
		return 1;

	if (!rt_create(&rt)) {
		printf("Error.\n");
		return 1;
	}

	if (!rt_register_cfunc(rt, "print", 1, params, ffi_print)) {
		printf("Failed to register an ffi func.\n");
		return 1;
	}

	if (!rt_register_source(rt, argv[1], text)) {
		printf("%s:%d: error: %s\n",
		       rt_get_file_name(rt),
		       rt_get_error_line(rt),
		       rt_get_error_message(rt));
		return 1;
	}

	if (!rt_call_with_name(rt, "main", 0, NULL, &ret)) {
		printf("%s:%d: error: %s\n",
		       rt_get_file_name(rt),
		       rt_get_error_line(rt),
		       rt_get_error_message(rt));
		return 1;
	}

	rt_shallow_gc(rt);
	rt_deep_gc(rt);

	printf("Execution succeeded.\n");
	return 0;
}

bool load_file(int argc, char *argv[])
{
	FILE *fp;
	size_t len;

	if (argc < 2) {
		printf("Specify a file.\n");
		return false;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		printf("Cannot open file.\n");
		return false;
	}

	len = fread(text, 1, sizeof(text) - 1, fp);
	if (len == 0) {
		printf("Cannot read the file.\n");
		return false;
	}
	text[len] = '\0';

	fclose(fp);

	return true;
}

bool ffi_print(struct rt_env *rt)
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
