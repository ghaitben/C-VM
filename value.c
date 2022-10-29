#include "value.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "error.h"

void freeValue(Value *value) {
		if(value->type == VALUE_TYPE_STRING) {
				free(value->as.string);
		}
}

Function *createFunction(char *name) {
		Function *function = malloc(sizeof(Function));
		function->name = name;
		function->local_top = 0;
		initByteArray(&function->code);
}

bool valueEquals(Value *this, Value *other) {
		if(this->type != other->type) return false;
		switch(this->type) {
				case VALUE_TYPE_NIL:
						 return true;
				case VALUE_TYPE_STRING:
						 return strcmp(this->as.string, other->as.string) == 0;
				case VALUE_TYPE_BOOLEAN:
						 return this->as.boolean == other->as.boolean;
		    case VALUE_TYPE_NUMBER:
						 return this->as.number == other->as.number;
		    default:
						 CHECK(false, "Unreachable state");
						 return false;
		}
}

void initValueArray(ValueArray *value_array) {
		value_array->count = 0;
		value_array->capacity = 0;
		value_array->array = NULL;
}

void freeValueArray(ValueArray *value_array) {
		// Free all the values inside the value_array before freeing the value_array itself.
		for(int i = 0; i < value_array->count; ++i) {
				freeValue(&value_array->array[i]);
		}
		free(value_array->array);
		initValueArray(value_array);
}

// Grow the array when the number of elements reaches the max capacity of the array.
uint8_t writeValueArray(ValueArray *value_array, Value value) {
		if(value_array->count + 1 > value_array->capacity) {
				value_array->capacity = value_array->capacity > 0 ? 2 * value_array->capacity : 8;
				value_array->array = realloc(value_array->array, sizeof(Value) * value_array->capacity);
		}
		value_array->array[value_array->count] = value;
		value_array->count++;
		return value_array->count - 1;
}
