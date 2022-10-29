#ifndef COMPILER_UTILITY_H
#define COMPILER_UTILITY_H

#include <stdint.h>

// The size of the stack of our virtual machine.
#define STACK_MAX 255

// The array where the bytecode will be stored before getting executed.
// The Bytecode is written to this array in the parsing phase.
typedef struct {
		int count;
		int capacity;
		uint8_t *array;
} ByteArray;
void initByteArray(ByteArray *byte_array);
void freeByteArray(ByteArray *byte_array);
void writeByteArray(ByteArray *byte_array, uint8_t byte);

// Local Variable representation
typedef struct {
		char *name;
		int scope;
} Local;

#endif
