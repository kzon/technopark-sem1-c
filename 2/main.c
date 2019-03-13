#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define STACK_INITIALIZER {NULL, 0, 0};
#define STACK_START_CAPACITY 10
#define STACK_CAPACITY_MULTIPLIER 2

typedef struct {
  char **values;
  size_t size;
  size_t capacity;
} stack;

bool stack_push(stack *stack, char *value);
char *stack_top(stack *stack);
char *stack_before_top(stack *stack);
void stack_pop(stack *stack);
bool stack_grow(stack *stack);
bool is_stack_empty(const stack *stack);
void stack_free(stack *stack);


#define STRING_INITIALIZER {NULL, 0, 0};
#define STRING_START_CAPACITY 50
#define STRING_CAPACITY_MULTIPLIER 2

typedef struct {
  char *content;
  size_t size;
  size_t capacity;
} string;

void string_clear(string *string);
bool string_add(string *string, char value);
bool string_grow(string *string);
bool string_grow_to(string *string, size_t new_capacity);
void string_free(string *string);

void print_error(); // todo

size_t get_operator_precedence(char operator);

void calculate_on_stacks(stack *operators_stack, stack *operands_stack);
void calculate(const char *first_operand, const char *second_operand, char operator, string *result);
void add(const char *first_operand, const char *second_operand, string *result);
void subtract(const char *first_operand, const char *second_operand, string *result);
void multiply(const char *first_operand, const char *second_operand, string *result);
void divide(const char *first_operand, const char *second_operand, string *result);

int main() {
  stack operators_stack = STACK_INITIALIZER;
  stack operands_stack = STACK_INITIALIZER;
  string input_number = STRING_INITIALIZER;
  char c;
  while (c = (char) getchar(), c != EOF) {
    if (c == '+' || c == '-' || c == '*' || c == '/') {
      stack_push(&operands_stack, input_number.content);
      string_clear(&input_number);
      calculate_on_stacks(&operators_stack, &operands_stack);
      char operator_as_string[2] = "\0";
      operator_as_string[0] = c;
      stack_push(&operators_stack, operator_as_string);
    } else if (isdigit(c)) {
      string_add(&input_number, c);
    }
  }

  stack_push(&operands_stack, input_number.content);
  calculate_on_stacks(&operators_stack, &operands_stack);
  printf("%s", stack_top(&operands_stack));

  stack_free(&operators_stack);
  stack_free(&operands_stack);
  string_free(&input_number);

  return 0;
}

bool stack_push(stack *stack, char *value) {
  assert(stack != NULL);
  if (stack->size + 1 >= stack->capacity && !stack_grow(stack))
    return false;
  stack->values[stack->size++] = strdup(value);
  return true;
}

char *stack_top(stack *stack) {
  assert(stack != NULL && stack->size > 0);
  return stack->values[stack->size - 1];
}

char *stack_before_top(stack *stack) {
  assert(stack != NULL && stack->size > 1);
  return stack->values[stack->size - 2];
}

void stack_pop(stack *stack) {
  assert(stack != NULL && stack->size > 0);
  free(stack->values[--stack->size]);
}

bool stack_grow(stack *stack) {
  assert(stack != NULL);
  size_t new_capacity = stack->capacity == 0
                        ? STACK_START_CAPACITY
                        : stack->capacity * STACK_CAPACITY_MULTIPLIER;
  char **new_content = realloc(stack->values, sizeof(char *) * new_capacity);
  if (new_content == NULL) return false;
  stack->values = new_content;
  stack->capacity = new_capacity;
  return true;
}

bool is_stack_empty(const stack *stack) {
  assert(stack != NULL);
  return stack->size == 0;
}

void stack_free(stack *stack) {
  assert(stack != NULL && stack->values != NULL);
  free(stack->values);
  while (stack->size > 0)
    free(stack->values[--stack->size]);
  stack->capacity = 0;
}

void string_clear(string *string) {
  assert(string != NULL);
  string->size = 0;
  string->content[0] = '\0';
}

bool string_add(string *string, char value) {
  assert(string != NULL);
  if (string->size + 1 >= string->capacity && !string_grow(string))
    return false;
  string->content[string->size++] = value;
  string->content[string->size] = '\0';
  return true;
}

bool string_grow(string *string) {
  assert(string != NULL);
  size_t new_capacity = string->capacity == 0
                        ? STRING_START_CAPACITY
                        : string->capacity * STRING_CAPACITY_MULTIPLIER;
  return string_grow_to(string, new_capacity);
}

bool string_grow_to(string *string, size_t new_capacity) {
  assert(string != NULL);
  char *new_content = realloc(string->content, sizeof(char) * new_capacity);
  if (new_content == NULL) return false;
  string->content = new_content;
  string->capacity = new_capacity;
  return true;
}

void string_free(string *string) {
  assert(string != NULL);
  if (string->content != NULL)
    free(string->content);
  string->size = string->capacity = 0;
}

inline void print_error() { printf("[error]"); }

size_t get_operator_precedence(char operator) {
  switch (operator) {
    case '+':
    case '-':return 1;

    case '*':
    case '/':return 2;

    case '^':return 3;
    default:return 0;
  }
}

void calculate_on_stacks(stack *operators_stack, stack *operands_stack) {
  string result = STRING_INITIALIZER;
  while (!is_stack_empty(operators_stack)) {
    char *operator = stack_top(operators_stack);
    char *second_operand = stack_top(operands_stack);
    char *first_operand = stack_before_top(operands_stack);

    calculate(first_operand, second_operand, operator[0], &result);
    printf("CALC %s %s %s = %s\n", first_operand, operator, second_operand, result.content);

    stack_pop(operators_stack);
    stack_pop(operands_stack);
    stack_pop(operands_stack);
    stack_push(operands_stack, result.content);
    string_clear(&result);
  }
  string_free(&result);
}

void calculate(const char *first_operand, const char *second_operand, char operator, string *result) {
  if (operator == '+')
    add(first_operand, second_operand, result);
  if (operator == '-')
    return subtract(first_operand, second_operand, result);
  if (operator == '*')
    return multiply(first_operand, second_operand, result);
  if (operator == '/')
    return divide(first_operand, second_operand, result);
}

void add(const char *first_operand, const char *second_operand, string *result) {
  size_t result_length = strlen(first_operand) + strlen(second_operand) + 1;
  string_grow_to(result, result_length);
  snprintf(result->content, result_length, "%d", atoi(first_operand) + atoi(second_operand));
}

void subtract(const char *first_operand, const char *second_operand, string *result) {
  size_t result_length = strlen(first_operand) + strlen(second_operand) + 1;
  string_grow_to(result, result_length);
  snprintf(result->content, result_length, "%d", atoi(first_operand) - atoi(second_operand));
}

void multiply(const char *first_operand, const char *second_operand, string *result) {
  size_t result_length = strlen(first_operand) + strlen(second_operand) + 1;
  string_grow_to(result, result_length);
  snprintf(result->content, result_length, "%d", atoi(first_operand) * atoi(second_operand));
}

void divide(const char *first_operand, const char *second_operand, string *result) {
  size_t result_length = strlen(first_operand) + strlen(second_operand) + 1;
  string_grow_to(result, result_length);
  snprintf(result->content, result_length, "%d", atoi(first_operand) / atoi(second_operand));
}