#ifndef COMPILER_VALUE_H
#define COMPILER_VALUE_H

#include <stdbool.h>
#include "utility.h"
#include <stdint.h>

// Creates a Value and initializes its number field with value.
#define CREATE_NUMBER(value) \
		((Value) {VALUE_TYPE_NUMBER, {.number = value}})

// Creates a Value and initializes its boolean field with value.
#define CREATE_BOOLEAN(value) \
		((Value) {VALUE_TYPE_BOOLEAN, {.boolean = value}})

// Creates a Value and initializes its string field with value of type (char *).
// Value owns the memory of the string field.
#define CREATE_STRING(value) \
		((Value) {VALUE_TYPE_STRING, {.string = value}})

// Creates a Value of type VALUE_TYPE_NIL that does not contain any data.
#define CREATE_NIL(value) \
		((Value) {VALUE_TYPE_NIL, {}})

// Creates a Value of type VALUE_TYPE_FUNCTION that contains data for a certain function
#define CREATE_FUNCTION(value) \
		((Value) {VALUE_TYPE_FUNCTION, {.function = value}})

#define IS_NUMBER(value) ((value).type == VALUE_TYPE_NUMBER)
#define IS_BOOLEAN(value) ((value).type == VALUE_TYPE_BOOLEAN)
#define IS_STRING(value) ((value).type == VALUE_TYPE_STRING)
#define IS_NIL(value) ((value).type == VALUE_TYPE_NIL)
#define IS_FUNCTION(value) ((value).type == VALUE_TYPE_FUNCTION)

typedef struct Value Value;
typedef struct ValueArray ValueArray;
typedef enum ValueType ValueType;

// ValueType defines the types supported for this language.
enum ValueType {
		VALUE_TYPE_NUMBER,
		VALUE_TYPE_NIL,
		VALUE_TYPE_BOOLEAN,
		VALUE_TYPE_STRING,
		VALUE_TYPE_FUNCTION
};

typedef struct {
		ByteArray code;
		Local locals[STACK_MAX];
		int local_top;
		int arity;
		char *name;
} Function;
Function *createFunction(char *name);
void freeFunction(Function *function);

typedef struct {
		Function *function;
		int ip;
		int fn_stack_top;
} CallFrame;

// Value is the runtime representation of our program's data.
// Data inside Value can be of any type among the types defined in the enum
// ValueType.
// Value owns the memory of heap allocated objects it may have.
struct Value {
		ValueType type;
		union {
				double number;
				bool boolean;
				char *string;
				Function *function;
		} as;
};
void freeValue(Value *value);
bool valueEquals(Value *this, Value *other);

// Array where all the values(i.e strings, numbers, booleans, ....) are stored.
// All the objects will remain in the array even if they are popped from the stack.
// This is in order to be able to free all the allocations after the program is done.
struct ValueArray {
		int count;
		int capacity;
		Value *array;
};
void initValueArray(ValueArray *value_array);
void freeValueArray(ValueArray *value_array);

// Returns the position of the inserted value in the value_array.
uint8_t writeValueArray(ValueArray *value_array, Value value);

extern Function *current_function;
extern Function *global_function;
extern CallFrame *current_frame;

#endif
