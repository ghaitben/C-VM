#include "vm.h"
#include "debug.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

static void decodeInstruction(OpCode op_code);
static void getHandler();

VM vm;
CallFrame *current_frame;

void initVM(VM *vm) {
		vm->stack_top = 0;
		vm->scope = 0;
		initValueArray(&vm->value_array);
		initHashTable(&vm->table);
}

void freeVM(VM *vm) {
		freeValueArray(&vm->value_array);
		freeHashTable(&vm->table);
		freeFunction(current_function);
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
		current_frame->ip++;
}

static void substractionHandler() {
		BINARY_OP(-, CREATE_NUMBER);
		current_frame->ip++;
}

static void multiplicationHandler() {
		BINARY_OP(*, CREATE_NUMBER);
		current_frame->ip++;
}

static void divisionHandler() {
		BINARY_OP(/, CREATE_NUMBER);
		current_frame->ip++;
}

static void valueHandler() {
		uint8_t pos_on_value_array = current_frame->function->code.array[current_frame->ip + 1];
		Value value = vm.value_array.array[pos_on_value_array];
		push(value);
		current_frame->ip += 2;
}

static void lessHandler() {
		BINARY_OP(<, CREATE_BOOLEAN);
		current_frame->ip++;
}

static void lessEqualHandler() {
		BINARY_OP(<=, CREATE_BOOLEAN);
		current_frame->ip++;
}

static void equalEqualHandler() {
		BINARY_OP(==, CREATE_BOOLEAN);
		current_frame->ip++;
} 

static void greaterHandler() {
		BINARY_OP(>, CREATE_BOOLEAN);
		current_frame->ip++;
}

static void greaterEqualHandler() {
		BINARY_OP(>=, CREATE_BOOLEAN);
		current_frame->ip++;
} 

static void bangEqualHandler() {
		BINARY_OP(!=, CREATE_BOOLEAN);
		current_frame->ip++;
}

static void notHandler() {
		CHECK(vm.stack_top > 0, "Trying to access an element from an Empty Stack!");
		Value *top = &vm.stack[vm.stack_top - 1];

		CHECK(IS_BOOLEAN(*top), "Operand of '!' operator must be a boolean!");
		top->as.boolean = !(top->as.boolean);
		current_frame->ip++;
}

static void negateHandler() {
		CHECK(vm.stack_top > 0, "Trying to access an element from an Empty Stack!");
		Value *top = &vm.stack[vm.stack_top - 1];

		CHECK(IS_NUMBER(*top), "Operand of '-' operator must be a number!");
		top->as.number = -(top->as.number);
		current_frame->ip++;
}

static void getHandler() {
		// OP_GET OP_VALUE index_on_value_array
		//   ^
		//  current_frame->ip

		uint8_t pos_on_value_array = current_frame->function->code.array[current_frame->ip + 2];
		uint8_t pos_on_stack = vm.value_array.array[pos_on_value_array].as.number;
		push(vm.stack[pos_on_stack]);
		current_frame->ip += 3;
}

static void assignHandler() {
		// OP_GET OP_VALUE index_on_value_array
		//   ^
		//  current_frame->ip

		uint8_t pos_on_value_array = current_frame->function->code.array[current_frame->ip + 2];
		uint8_t pos_on_stack = vm.value_array.array[pos_on_value_array].as.number;
		vm.stack[pos_on_stack] = vm.stack[vm.stack_top - 1];
		current_frame->ip += 3;
}

static bool isTrue(Value value) {
		if(value.type == VALUE_TYPE_NIL) return false;
		if(value.type == VALUE_TYPE_BOOLEAN) return value.as.boolean;
		return true;
}

static void jumpIfFalseHandler() {
		uint16_t jump_size = (current_frame->function->code.array[current_frame->ip + 1] << 8) 
				| current_frame->function->code.array[current_frame->ip + 2];
		current_frame->ip += !isTrue(pop()) ? jump_size : 3;
}

static void jumpHandler() {
		uint16_t jump_size = (current_frame->function->code.array[current_frame->ip + 1] << 8)
				| current_frame->function->code.array[current_frame->ip + 2];
		current_frame->ip += jump_size;
}

static void jumpBackwardHandler() {
		uint16_t jump_size = (current_frame->function->code.array[current_frame->ip + 1] << 8)
				| current_frame->function->code.array[current_frame->ip + 2];
		current_frame->ip -= jump_size;
}

// This is where our virtual machine will spend most of its time.
// Our VM first reads the instruction from the array `code` (field in the VM struct), 
// and then tries to decode it by executing this function.
// Each operation/instruction has a specific handler that takes care of modifying the state of 
// the stack and incrementing our VM's instruction pointer `current_frame->ip`.
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
				case OP_GET:
						getHandler();
						break;
				case OP_ASSIGN:
						assignHandler();
						break;
				case OP_POP:
						pop();
						current_frame->ip++;
						break;
				case OP_JUMP_IF_FALSE:
						jumpIfFalseHandler();
						break;
				case OP_JUMP:
						jumpHandler();
						break;
				case OP_JUMP_BACKWARD:
						jumpBackwardHandler();
						break;
		}
}

void decode() {
		current_frame = &vm.frames[0];
		current_frame->function = current_function;
		current_frame->ip = 0;
		current_frame->fun_stack_top = 0;

		while(current_frame->ip < current_frame->function->code.count) {
				uint8_t instruction = current_frame->function->code.array[current_frame->ip];
				decodeInstruction(instruction);
		}
}
