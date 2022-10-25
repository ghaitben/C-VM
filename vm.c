#include "vm.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

static void decodeInstruction(OpCode op_code);

VM vm;

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

void initVM(VM *vm) {
		vm->stack_top = 0;
		vm->ip = 0;
		initByteArray(&vm->code);
		initValueArray(&vm->value_array);
}

void freeVM(VM *vm) {
		freeByteArray(&vm->code);
		freeValueArray(&vm->value_array);
		initVM(vm);
}

void push(Value value) {
		CHECK(vm.stack_top < STACK_MAX, "Stack Overflow!");
		vm.stack[vm.stack_top++] = value;
}

Value pop() {
		CHECK(vm.stack_top > 0, "Trying to pop an element from an empty Stack!");
		vm.stack_top--;
		return vm.stack[vm.stack_top];
}

static void additionHandler() {
		Value rhs = pop();
		Value lhs = pop();

		CHECK((IS_NUMBER(lhs) && IS_NUMBER(rhs)) ||
						(IS_STRING(lhs) && IS_STRING(rhs)), "Both Operands of '+' must be numbers or strings.");

		if(IS_NUMBER(lhs) && IS_NUMBER(rhs)) {
				push(CREATE_NUMBER(lhs.as.number + rhs.as.number));
		}
		else {
				char *lhs_string = lhs.as.string;
				char *rhs_string = rhs.as.string;
				
				size_t concat_size = strlen(lhs_string) + strlen(rhs_string);
				char *concat_string = malloc(concat_size + 1);

				memcpy(concat_string, lhs_string, strlen(lhs_string));
				memcpy(concat_string + strlen(lhs_string), rhs_string, strlen(rhs_string));
				concat_string[concat_size] = '\0';
				
				Value new_value = CREATE_STRING(concat_string);
				push(new_value);
				// Push to value array in order for the new value to be freed at the end
				// of the program.
				writeValueArray(&vm.value_array, new_value);
		}
		vm.ip++;
}

static void substractionHandler() {
		BINARY_OP(-, CREATE_NUMBER);
		vm.ip++;
}

static void multiplicationHandler() {
		BINARY_OP(*, CREATE_NUMBER);
		vm.ip++;
}

static void divisionHandler() {
		BINARY_OP(/, CREATE_NUMBER);
		vm.ip++;
}

static void valueHandler() {
		uint8_t pos_on_value_array = vm.code.array[vm.ip + 1];
		Value value = vm.value_array.array[pos_on_value_array];
		push(value);
		vm.ip += 2;
}

static void lessHandler() {
		BINARY_OP(<, CREATE_BOOLEAN);
		vm.ip++;
}

static void lessEqualHandler() {
		BINARY_OP(<=, CREATE_BOOLEAN);
		vm.ip++;
}

static void equalEqualHandler() {
		BINARY_OP(==, CREATE_BOOLEAN);
		vm.ip++;
} 

static void greaterHandler() {
		BINARY_OP(>, CREATE_BOOLEAN);
		vm.ip++;
}

static void greaterEqualHandler() {
		BINARY_OP(>=, CREATE_BOOLEAN);
		vm.ip++;
} 

static void bangEqualHandler() {
		BINARY_OP(!=, CREATE_BOOLEAN);
		vm.ip++;
}

static void notHandler() {
		CHECK(vm.stack_top > 0, "Trying to access an element from an Empty Stack!");
		Value *top = &vm.stack[vm.stack_top - 1];

		CHECK(IS_BOOLEAN(*top), "Operand of '!' operator must be a boolean!");
		top->as.boolean = !(top->as.boolean);
		vm.ip++;
}

static void negateHandler() {
		CHECK(vm.stack_top > 0, "Trying to access an element from an Empty Stack!");
		Value *top = &vm.stack[vm.stack_top - 1];

		CHECK(IS_NUMBER(*top), "Operand of '-' operator must be a number!");
		top->as.number = -(top->as.number);
		vm.ip++;
}

// This is where our virtual machine will spend most of its time.
// Our VM first reads the instruction from the array `code` (field in the VM struct), 
// and then tries to decode it by executing this function.
// Each operation/instruction has a specific handler that takes care of modifying the state of 
// the stack and incrementing our VM's instruction pointer `vm.ip`.
static void decodeInstruction(OpCode op) {
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
				case OP_LESS:
						lessHandler();
						break;
				case OP_LESS_EQUAL:
						lessEqualHandler();
						break;
				case OP_EQUAL_EQUAL:
						equalEqualHandler();
						break;
				case OP_GREATER:
						greaterHandler();
						break;
				case OP_GREATER_EQUAL:
						greaterEqualHandler();
						break;
				case OP_BANG_EQUAL:
						bangEqualHandler();
						break;
				case OP_NOT:
						notHandler();
						break;
				case OP_NEGATE:
						negateHandler();
						break;
		}
}

void decode() {
		while(vm.ip < vm.code.count) {
				uint8_t instruction = vm.code.array[vm.ip];
				decodeInstruction(instruction);
		}
}
