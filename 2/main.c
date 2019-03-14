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
bool stack_push_char(stack *stack, char value);
char *stack_top(stack *stack);
char *stack_before_top(stack *stack);
char stack_top_char(stack *stack);
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
bool string_append(string *string, const char *other);
bool string_grow(string *string);
bool string_grow_to(string *string, size_t new_capacity);
bool is_string_empty(string *string);
void string_free(string *string);

bool evaluate_expression(string *expression, string *result);
bool calculate_expression_on_stack(stack *operators_stack, stack *operands_stack);
void calculate(const char *first_operand, const char *second_operand, char operator, string *result);
void add(const char *first_operand, const char *second_operand, string *result);
void subtract(const char *first_operand, const char *second_operand, string *result);
void multiply(const char *first_operand, const char *second_operand, string *result);
void divide(const char *first_operand, const char *second_operand, string *result);
size_t get_operator_precedence(char operator);

void print_error();

char *strdup(const char *s);

int main() {
  string expression = STRING_INITIALIZER;
  char input_buffer[1024];
  while (fgets(input_buffer, 1024, stdin) != NULL)
    string_append(&expression, input_buffer);
  string result = STRING_INITIALIZER;
  if (evaluate_expression(&expression, &result))
    printf("%s\n", result.content);
  else
    print_error();

  string_free(&expression);
  string_free(&result);

  return 0;
}

bool evaluate_expression(string *expression, string *result) {
  stack operators = STACK_INITIALIZER;
  stack operands = STACK_INITIALIZER;
  string number = STRING_INITIALIZER;
  char current;
  size_t i = 0;
  bool success = true;
  while (current = expression->content[i++], current != '\0') {
    if (isdigit(current)) {
      string_add(&number, current);
    } else if (current == '+' || current == '-' || current == '*' || current == '/') {
      if (!is_string_empty(&number)) {
        stack_push(&operands, number.content);
        string_clear(&number);
      }
      if (is_stack_empty(&operators) || stack_top_char(&operators) == '('
          || get_operator_precedence(current) > get_operator_precedence(stack_top_char(&operators))) {
        stack_push_char(&operators, current);
      } else {
        while (success && !is_stack_empty(&operators) && stack_top_char(&operators) != '('
            && get_operator_precedence(stack_top_char(&operators)) >= get_operator_precedence(current))
          success = calculate_expression_on_stack(&operators, &operands);
        if (!success) break;
        stack_push_char(&operators, current);
      }
    } else if (current == '(') {
      stack_push_char(&operators, current);
    } else if (current == ')') {
      if (!is_string_empty(&number)) {
        stack_push(&operands, number.content);
        string_clear(&number);
      }
      while (success && !is_stack_empty(&operators) && stack_top_char(&operators) != '(')
        success = calculate_expression_on_stack(&operators, &operands);
      if (!success) break;
      if (is_stack_empty(&operators))
        success = false;
      else
        stack_pop(&operators);
    }
  }

  if (success) {
    if (!is_string_empty(&number))
      stack_push(&operands, number.content);
    while (success && !is_stack_empty(&operators))
      success = calculate_expression_on_stack(&operators, &operands);
    if (success)
      string_append(result, stack_top(&operands));
  }

  stack_free(&operators);
  stack_free(&operands);
  string_free(&number);
  return success;
}

bool stack_push(stack *stack, char *value) {
  assert(stack != NULL);
  if (stack->size + 1 >= stack->capacity && !stack_grow(stack))
    return false;
  stack->values[stack->size++] = strdup(value);
  return true;
}

bool stack_push_char(stack *stack, char value) {
  char value_as_string[2] = "\0";
  value_as_string[0] = value;
  return stack_push(stack, value_as_string);
}

char *stack_top(stack *stack) {
  assert(stack != NULL && stack->size > 0);
  return stack->values[stack->size - 1];
}

char stack_top_char(stack *stack) {
  return stack_top(stack)[0];
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
  assert(stack != NULL);
  if (stack->values != NULL) {
    while (stack->size > 0)
      free(stack->values[--stack->size]);
    free(stack->values);
  }
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

bool string_append(string *string, const char *other) {
  assert(string != NULL && other != NULL);
  size_t other_length = strlen(other);
  if (string->size + other_length >= string->capacity) {
    size_t new_capacity = string->capacity +
        other_length * (1 + STRING_CAPACITY_MULTIPLIER) / STRING_CAPACITY_MULTIPLIER;
    if (!string_grow_to(string, new_capacity)) return false;
  }
  strncpy(string->content + string->size, other, other_length);
  string->size += other_length;
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

bool is_string_empty(string *string) {
  assert(string != NULL);
  return string->size == 0;
}

void string_free(string *string) {
  assert(string != NULL);
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
    default:return 0;
  }
}

bool calculate_expression_on_stack(stack *operators_stack, stack *operands_stack) {
  string result = STRING_INITIALIZER;
  char operator = stack_top_char(operators_stack);
  if (operator == '(') return false;
  char *second_operand = stack_top(operands_stack);
  char *first_operand = stack_before_top(operands_stack);

  calculate(first_operand, second_operand, operator, &result);

  stack_pop(operators_stack);
  stack_pop(operands_stack);
  stack_pop(operands_stack);
  stack_push(operands_stack, result.content);
  string_free(&result);
  return true;
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

char *strdup(const char *s) {
  size_t len = strlen(s) + 1;
  void *new = malloc(len);
  if (new == NULL)
    return NULL;
  return (char *) memcpy(new, s, len);
}