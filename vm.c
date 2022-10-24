#include "vm.h"
#include "error.h"
#include <stdlib.h>

VM vm;

void initByteArray(ByteArray *byte_array) {
		byte_array->count = 0;
		byte_array->capacity = 1;
		byte_array->array = NULL;
}

void freeByteArray(ByteArray *byte_array) {
		free(byte_array->array);
}

void writeByteArray(ByteArray *byte_array, uint8_t byte) {
		if(byte_array->count + 1 > byte_array->capacity) {
				byte_array->capacity = byte_array->capacity > 0 ? 2 * byte_array->capacity : 8;
				byte_array->array = realloc(byte_array->array, /*sizeof byte =*/1 * byte_array->capacity);
		}
		byte_array->array[byte_array->count] = byte;
		byte_array->count++;
}

void initVM(VM *vm) {
		vm->stack_top = 0;
		vm->ip = 0;
		initByteArray(&vm->code);
}

void freeVM(VM *vm) {
		freeByteArray(&vm->code);
}

void push(Value value) {
		CHECK(vm.stack_top < STACK_MAX, "Stack Overflow!");
		vm.stack[vm.stack_top++] = value;
}

Value pop() {
		CHECK(vm.stack_top > 0, "Trying to pop an element from an empty Stack!");
		vm.stack_top--;
}

static void additionHandler() {
		BINARY_OP(+);
		vm.ip++;
}

static void substractionHandler() {
		BINARY_OP(-);
		vm.ip++;
}

static void multiplicationHandler() {
		BINARY_OP(*);
		vm.ip++;
}

static void divisionHandler() {
		BINARY_OP(/);
		vm.ip++;
}

static void valueHandler() {
		uint8_t pos_on_value_array = vm.code.array[vm.ip + 1];
		Value value = vm.value_array.array[pos_on_value_array];
		push(value);
		vm.ip += 2;
}

void decode(OpCode op) {
		switch(op) {
				case OP_ADD:
						additionHandler();
						break;
				case OP_SUBSTRACT:
						substractionHandler();
						break;
				case OP_MULTIPLY:
						multiplicationHandler();
						break;
				case OP_DIVIDE:
						divisionHandler();
						break;
				case OP_VALUE:
						valueHandler();
						break;
		}
}
