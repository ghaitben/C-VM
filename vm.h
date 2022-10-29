#ifndef COMPILER_VM_H
#define COMPILER_VM_H

#include "value.h"
#include "utility.h"
#include "hash_table.h"
#include <stdint.h>

/*
 * This macro performs a binary operation using the operator `op` and
 * pushes the result into the stack. The type of the result is based on the parameter 
 * `value_type`.
 * The macro is wrapped around a do while loop (that executes once) in order to avoid any 
 * scoping problems and other obscure bugs.
 * We check whether the two operands of the operator `op` are numbers, otherwise we report an error
 * and exit the system.
 * */
#define BINARY_OP(op, value_type) \
		do { \
				Value rhs = pop(); \
				Value lhs = pop(); \
				CHECK(IS_NUMBER(lhs) && IS_NUMBER(rhs), "Both Operands must be numbers"); \
				push(value_type(lhs.as.number op rhs.as.number)); \
		} while(false)

typedef struct VM VM;

// The operations that our virtual machine can decode and execute.
typedef enum {
		OP_ADD,
		OP_SUBSTRACT,
		OP_NEGATE,
		OP_JUMP_IF_FALSE,
		OP_JUMP_BACKWARD,
		OP_CALL,
		OP_JUMP,
		OP_NOT,
		OP_MULTIPLY,
		OP_CHECK_REFLEXIVE_ASSIGNMENT,
		OP_DIVIDE,
		OP_PRINT,
		OP_ASSIGN,
		OP_VALUE,
		OP_LESS,
		OP_SET,
		OP_GET,
		OP_LESS_EQUAL,
		OP_POP,
		OP_GREATER,
		OP_GREATER_EQUAL,
		OP_EQUAL_EQUAL,
		OP_BANG_EQUAL
} OpCode;

// There will be one global instance of the virtual machine throughout the whole process.
struct VM {
		CallFrame frames[STACK_MAX];
		int frame_top;

		Value stack[STACK_MAX];
		int stack_top;

		ValueArray value_array;
		HashTable table;
		int scope;
};

void initVM(VM *vm);
void freeVM(VM *vm);
// Push a value onto the stack of our virtual machine
void push(Value value);
// Pops the last value pushed into the stack of our virtual machine and
// returns it.
Value pop();

// Interpret the bytecode written in the ByteArray.
void interpret();
void decode();

extern VM vm;

#endif
