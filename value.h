#ifndef COMPILER_VALUE_H
#define COMPILER_VALUE_H

#include <stdbool.h>
#include <stdint.h>

// Creates a Value and initializes its number field with value.
#define CREATE_NUMBER(value) \
		((Value) {VALUE_TYPE_NUMBER, {.number = value}})

// Creates a Value and initializes its boolean field with value.
#define CREATE_BOOLEAN(value) \
		((Value) {VALUE_TYPE_BOOLEAN, {.boolean = value}})

#define IS_NUMBER(value) ((value).type == VALUE_TYPE_NUMBER)
#define IS_BOOLEAN(value) ((value).type == VALUE_TYPE_BOOLEAN)

typedef struct Value Value;
typedef struct ValueArray ValueArray;
typedef enum ValueType ValueType;

// ValueType defines the types supported for this language.
enum ValueType {
		VALUE_TYPE_NUMBER,
		VALUE_TYPE_BOOLEAN
};

// Value is the runtime representation of our program's data.
// Data inside Value can be of any type among the types defined in the enum
// ValueType.
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

// Returns the position of the inserted value in the value_array.
uint8_t writeValueArray(ValueArray *value_array, Value value);

#endif
