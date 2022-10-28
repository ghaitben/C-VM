#ifndef COMPILER_VM_H
#define COMPILER_VM_H

#include "value.h"
#include "hash_table.h"
#include <stdint.h>

// The size of the stack of our virtual machine.
#define STACK_MAX 255

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

#define WRITE_VALUE(value_type, ...) \
		do { \
				writeByteArray(&vm.code, OP_VALUE); \
				uint8_t pos_on_value_array = writeValueArray(&vm.value_array, value_type(__VA_ARGS__)); \
				writeByteArray(&vm.code, pos_on_value_array); \
		}while(false)

typedef struct VM VM;
typedef struct ByteArray ByteArray;

// The operations that our virtual machine can decode and execute.
typedef enum {
		OP_ADD,
		OP_SUBSTRACT,
		OP_NEGATE,
		OP_JUMP_IF_FALSE,
		OP_JUMP,
		OP_NOT,
		OP_MULTIPLY,
		OP_DIVIDE,
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

// The array where the bytecode will be stored before getting executed.
// The Bytecode is written to this array in the parsing phase.
struct ByteArray {
		int count;
		int capacity;
		uint8_t *array;
};
void initByteArray(ByteArray *byte_array);
void freeByteArray(ByteArray *byte_array);
void writeByteArray(ByteArray *byte_array, uint8_t byte);

typedef struct {
		char *name;
		int scope;
} Local;

// There will be one global instance of the virtual machine throughout the whole process.
struct VM {
		Value stack[STACK_MAX];
		Local locals[STACK_MAX];
		int stack_top;
		int local_top;
		int ip;
		ByteArray code;
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
void decode();

extern VM vm;

#endif
