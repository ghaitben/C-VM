#ifndef COMPILER_ERROR_H
#define COMPILER_ERROR_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/*
 * The macro CHECK works like an assertion. It will exit the system if the input condition
 * is not verified.
 * It also displays the filename and the line number of where the error occured exactly in the code.
 * */

#define CHECK(condition, message) \
		CHECK_HELPER(condition, message, __FILE__, __LINE__)

#define CHECK_HELPER(condition, message, filename, line) \
		check(condition, message, filename, line)

static void check(bool condition, const char *message, const char *filename, int line) {
		if(condition) return;
		fprintf(stderr, "[File : %s][Line : %d] %s\n",
						filename, line, message);
		exit(-1);
}

#endif
