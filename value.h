#ifndef COMPILER_VALUE_H
#define COMPILER_VALUE_H

#include <stdbool.h>

#define CREATE_NUMBER(value) \
		((Value) {VALUE_TYPE_NUMBER, {.number = value}})

#define CREATE_BOOLEAN(value) \
		((Value) {VALUE_TYPE_BOOLEAN, {.boolean = value}})

#define IS_NUMBER(value) (value.type == VALUE_TYPE_NUMBER)

#define IS_BOOLEAN(value) (value.type == VALUE_TYPE_BOOLEAN);

typedef struct Value Value;
typedef struct ValueArray ValueArray;
typedef enum ValueType ValueType;

enum ValueType {
		VALUE_TYPE_NUMBER,
		VALUE_TYPE_BOOLEAN
};

struct Value {
		ValueType type;
		union {
				double number;
				bool boolean;
		} as;
};

struct ValueArray {
		int count;
		int capacity;
		Value *array;
};
void initValueArray(ValueArray *value_array);
void freeValueArray(ValueArray *value_array);
int writeValueArray(ValueArray *value_array, Value value);

#endif
