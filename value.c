#include "value.h"
#include <stddef.h>
#include <stdlib.h>

void initValueArray(ValueArray *value_array) {
		value_array->count = 0;
		value_array->capacity = 0;
		value_array->array = NULL;
}

int writeValueArray(ValueArray *value_array, Value value) {
		if(value_array->count + 1 > value_array->capacity) {
				value_array->capacity = value_array->capacity > 0 ? 2 * value_array->capacity : 8;
				value_array->array = realloc(value_array->array, /*sizeof byte =*/1 * value_array->capacity);
		}
		value_array->array[value_array->count] = value;
		value_array->count++;
		return value_array->count - 1;
}