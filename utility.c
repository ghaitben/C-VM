#include "utility.h"
#include <stddef.h>
#include <stdlib.h>

void initByteArray(ByteArray *byte_array) {
		byte_array->count = 0;
		byte_array->capacity = 0;
		byte_array->array = NULL;
}

void freeByteArray(ByteArray *byte_array) {
		free(byte_array->array);
		initByteArray(byte_array);
}

void writeByteArray(ByteArray *byte_array, uint8_t byte) {
		if(byte_array->count + 1 > byte_array->capacity) {
				byte_array->capacity = byte_array->capacity > 0 ? 2 * byte_array->capacity : 8;
				byte_array->array = realloc(byte_array->array, /*sizeof byte =*/1 * byte_array->capacity);
		}
		byte_array->array[byte_array->count] = byte;
		byte_array->count++;
}
