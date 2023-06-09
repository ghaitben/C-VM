#include "value.h"
#include "error.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void freeValue(Value *value) {
  switch (value->type) {
  case VALUE_TYPE_STRING:
    free(value->as.string);
    break;
  case VALUE_TYPE_FUNCTION:
    freeFunction(value->as.function);
    break;
  }
}

void freeFunction(Function *function) {
  freeByteArray(&function->code);
  for (int i = 0; i < function->local_top; ++i) {
    free(function->locals[i].name);
  }
  free(function);
}

Function *createFunction(char *name) {
  Function *function = malloc(sizeof(Function));
  function->name = name;
  function->local_top = 0;
  function->arity = 0;
  initByteArray(&function->code);
}

void initClosure(Closure *closure) {
  closure->function = NULL;
  initClosureArray(&closure->closure_array);
}

void freeClosure(Closure *closure) {
  freeFunction(closure->function);
  freeClosureArray(&closure->closure_array);
  initClosure(closure);
}

Closure *createClosure(char *function_name) {
  Closure *closure = malloc(sizeof(Closure));

  closure->function = createFunction(function_name);
  initClosure(closure);
  return closure;
}

void initClosureArray(ClosureArray *closure_array) {
  closure_array->count = 0;
  closure_array->capacity = 0;
  closure_array->array = NULL;
}

void freeClosureArray(ClosureArray *closure_array) {}

void writeClosureArray(ClosureArray *closure_array, uint8_t stack_pos) {
  if (closure_array->count + 1 > closure_array->capacity) {
    closure_array->capacity = get_new_capacity(closure_array->capacity);
        closure_array->capacity > 0 ? 2 * closure_array->capacity : 8;
    closure_array->array =
        realloc(closure_array->array,
                sizeof(*closure_array->array) * closure_array->capacity);
  }
  closure_array->array[closure_array->count].stack_pos = stack_pos;
  closure_array->count++;
}

bool valueEquals(Value *this, Value *other) {

  if (this->type != other->type)
    return false;
  switch (this->type) {
  case VALUE_TYPE_NIL:
    return true;
  case VALUE_TYPE_STRING:
    return strcmp(this->as.string, other->as.string) == 0;
  case VALUE_TYPE_BOOLEAN:
    return this->as.boolean == other->as.boolean;
  case VALUE_TYPE_NUMBER:
    return this->as.number == other->as.number;
  case VALUE_TYPE_FUNCTION:
    return !strcmp(this->as.function->name, other->as.function->name);
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
  // Free all the values inside the value_array before freeing the value_array
  // itself.
  for (int i = 0; i < value_array->count; ++i) {
    freeValue(&value_array->array[i]);
  }
  free(value_array->array);
  initValueArray(value_array);
}

// Grow the array when the number of elements reaches the max capacity of the
// array.
uint8_t writeValueArray(ValueArray *value_array, Value value) {
  // Check if this value is already in the array
  for (int i = 0; i < value_array->count; ++i) {
    if (valueEquals(&value_array->array[i], &value))
      return i;
  }

  if (value_array->count + 1 > value_array->capacity) {
    value_array->capacity =
        value_array->capacity > 0 ? 2 * value_array->capacity : 8;
    value_array->array =
        realloc(value_array->array, sizeof(Value) * value_array->capacity);
  }
  value_array->array[value_array->count] = value;
  value_array->count++;
  return value_array->count - 1;
}
