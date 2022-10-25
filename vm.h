#ifndef COMPILER_VM_H
#define COMPILER_VM_H

#include "value.h"
#include <stdint.h>

#define STACK_MAX 255

#define BINARY_OP(op, value_type) \
		do { \
				Value rhs = pop(); \
				Value lhs = pop(); \
				CHECK(IS_NUMBER(lhs) && IS_NUMBER(rhs), "Both Operands must be numbers"); \
				push(value_type(lhs.as.number op rhs.as.number)); \
		} while(false)

typedef struct VM VM;
typedef struct ByteArray ByteArray;
typedef enum OpCode OpCode;

enum OpCode {
		OP_ADD,
		OP_SUBSTRACT,
		OP_NEGATE,
		OP_NOT,
		OP_MULTIPLY,
		OP_DIVIDE,
		OP_VALUE,
		OP_LESS,
		OP_LESS_EQUAL,
		OP_GREATER,
		OP_GREATER_EQUAL,
		OP_EQUAL_EQUAL,
		OP_BANG_EQUAL
};

struct ByteArray {
		int count;
		int capacity;
		uint8_t *array;
};
void initByteArray(ByteArray *byte_array);
void freeByteArray(ByteArray *byte_array);
void writeByteArray(ByteArray *byte_array, uint8_t byte);

struct VM {
		Value stack[STACK_MAX];
		int stack_top;
		int ip;
		ByteArray code;
		ValueArray value_array;
};

void initVM(VM *vm);
void freeVM(VM *vm);
void push(Value value);
Value pop();
void decode();

extern VM vm;

#endif
