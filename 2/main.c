#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

#define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define INPUT_BUFFER_SIZE 1024

#define STRING_INITIALIZER {NULL, 0, 0}
#define STRING_START_CAPACITY 50
#define STRING_CAPACITY_MULTIPLIER 2

typedef struct {
  char *content;
  size_t size;
  size_t capacity;
} string;

void string_clear(string *string);
void string_add(string *string, char value);
void string_append(string *string, const char *other);
void string_grow(string *string);
void string_grow_to(string *string, size_t new_capacity);
bool is_string_empty(const string *string);
void string_free(string *string);

#define NUMBER_PART_SIZE 9
#define NUMBER_PART_FORMAT "%09d"
#define NUMBER_BASE (int)1E9
#define NUMBER_START_PARTS_CAPACITY 10
#define NUMBER_PARTS_CAPACITY_MULTIPLIER 2

typedef struct {
  int *parts;
  size_t parts_size;
  size_t parts_capacity;
  bool is_negative;
} number;

number *number_new(size_t parts_initial_capacity);
number *number_zero();
number *number_from_string(const char *string);
number *number_from_int(int value);
number *number_from_number(const number *source);
void number_append_part(number *number, int part);
void number_parts_grow(number *number);
void number_parts_grow_to(number *number, size_t new_capacity);
void number_remove_leading_zeroes(number *number);
void number_sprint(const number *number, string *destination);
void number_free(number *number);
bool is_numbers_equal(const number *first, const number *second);
bool is_numbers_less(const number *first, const number *second);
bool is_numbers_abs_less(const number *first, const number *second);

// Calculations can modify its params for performance purpose
number *number_add(number *first, number *second);
number *number_subtract(number *first, number *second);
number *number_multiply(number *first, number *second);
number *number_divide(number *first, number *second);

#define STACK_INITIALIZER {NULL, 0, 0}
#define STACK_START_CAPACITY 10
#define STACK_CAPACITY_MULTIPLIER 2

typedef struct {
  number **values;
  size_t size;
  size_t capacity;
} number_stack;

void number_stack_push(number_stack *stack, const number *number);
void number_stack_push_string(number_stack *stack, const char *number_as_string);
number *number_stack_top(const number_stack *stack);
number *number_stack_before_top(const number_stack *stack);
void number_stack_pop(number_stack *stack);
void number_stack_grow(number_stack *stack);
void number_stack_free(number_stack *stack);

typedef struct {
  char *values;
  size_t size;
  size_t capacity;
} char_stack;

void char_stack_push(char_stack *stack, char value);
char char_stack_top(const char_stack *stack);
void char_stack_pop(char_stack *stack);
void char_stack_grow(char_stack *stack);
bool is_char_stack_empty(const char_stack *stack);
void char_stack_free(char_stack *stack);

bool evaluate_expression(const string *expression, string *result);
bool calculate_expression_on_stack_top(char_stack *operators, number_stack *operands);
number *calculate(number *first, number *second, char operator);
bool is_operator(char c);
size_t get_operator_precedence(char operator);

void print_error();

char *strdup(const char *string);

int main() {
  string expression = STRING_INITIALIZER;
  char input_buffer[INPUT_BUFFER_SIZE];
  while (fgets(input_buffer, INPUT_BUFFER_SIZE, stdin) != NULL)
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

void number_stack_push(number_stack *stack, const number *number) {
  assert(stack != NULL);
  if (stack->size + 1 >= stack->capacity)
    number_stack_grow(stack);
  stack->values[stack->size++] = number_from_number(number);
}

void number_stack_push_string(number_stack *stack, const char *number_as_string) {
  assert(stack != NULL);
  if (stack->size + 1 >= stack->capacity)
    number_stack_grow(stack);
  stack->values[stack->size++] = number_from_string(number_as_string);
}

number *number_stack_top(const number_stack *stack) {
  assert(stack != NULL && stack->size > 0);
  return stack->values[stack->size - 1];
}

number *number_stack_before_top(const number_stack *stack) {
  assert(stack != NULL && stack->size > 1);
  return stack->values[stack->size - 2];
}

void number_stack_pop(number_stack *stack) {
  assert(stack != NULL && stack->size > 0);
  number_free(stack->values[--stack->size]);
}

void number_stack_grow(number_stack *stack) {
  assert(stack != NULL);
  size_t new_capacity = stack->capacity == 0
                        ? STACK_START_CAPACITY
                        : stack->capacity * STACK_CAPACITY_MULTIPLIER;
  number **new_values = realloc(stack->values, sizeof(number *) * new_capacity);
  assert(new_values != NULL);
  stack->values = new_values;
  stack->capacity = new_capacity;
}

void number_stack_free(number_stack *stack) {
  assert(stack != NULL);
  if (stack->values != NULL) {
    while (stack->size > 0)
      number_stack_pop(stack);
    free(stack->values);
  }
  stack->size = stack->capacity = 0;
}

void char_stack_push(char_stack *stack, char value) {
  assert(stack != NULL);
  if (stack->size + 1 >= stack->capacity)
    char_stack_grow(stack);
  stack->values[stack->size++] = value;
}

char char_stack_top(const char_stack *stack) {
  assert(stack != NULL && stack->size > 0);
  return stack->values[stack->size - 1];
}

void char_stack_pop(char_stack *stack) {
  assert(stack != NULL && stack->size > 0);
  --stack->size;
}

void char_stack_grow(char_stack *stack) {
  assert(stack != NULL);
  size_t new_capacity = stack->capacity == 0
                        ? STACK_START_CAPACITY
                        : stack->capacity * STACK_CAPACITY_MULTIPLIER;
  char *new_values = realloc(stack->values, sizeof(char) * new_capacity);
  assert(new_values != NULL);
  stack->values = new_values;
  stack->capacity = new_capacity;
}

bool is_char_stack_empty(const char_stack *stack) {
  assert(stack != NULL);
  return stack->size == 0;
}

void char_stack_free(char_stack *stack) {
  assert(stack != NULL);
  free(stack->values);
  stack->size = stack->capacity = 0;
}

void string_clear(string *string) {
  assert(string != NULL);
  string->size = 0;
  string->content[0] = '\0';
}

void string_add(string *string, char value) {
  assert(string != NULL);
  if (string->size + 1 >= string->capacity)
    string_grow(string);
  string->content[string->size++] = value;
  string->content[string->size] = '\0';
}

void string_append(string *string, const char *other) {
  assert(string != NULL && other != NULL);
  size_t other_length = strlen(other);
  if (string->size + other_length >= string->capacity) {
    size_t new_capacity = string->capacity +
        other_length * (1 + STRING_CAPACITY_MULTIPLIER) / STRING_CAPACITY_MULTIPLIER;
    string_grow_to(string, new_capacity);
  }
  strncpy(string->content + string->size, other, other_length);
  string->size += other_length;
  string->content[string->size] = '\0';
}

void string_grow(string *string) {
  assert(string != NULL);
  size_t new_capacity = string->capacity == 0
                        ? STRING_START_CAPACITY
                        : string->capacity * STRING_CAPACITY_MULTIPLIER;
  string_grow_to(string, new_capacity);
}

void string_grow_to(string *string, size_t new_capacity) {
  assert(string != NULL);
  char *new_content = realloc(string->content, sizeof(char) * new_capacity);
  assert(new_content != NULL);
  string->content = new_content;
  string->capacity = new_capacity;
}

bool is_string_empty(const string *string) {
  assert(string != NULL);
  return string->size == 0;
}

void string_free(string *string) {
  assert(string != NULL);
  free(string->content);
  string->size = string->capacity = 0;
}

number *number_new(size_t parts_initial_capacity) {
  number *new_number = malloc(sizeof(number));
  assert(new_number != NULL);
  new_number->parts_size = 0;
  if (parts_initial_capacity != 0) {
    new_number->parts = malloc(sizeof(int) * parts_initial_capacity);
    assert(new_number->parts != NULL);
  } else {
    new_number->parts = NULL;
  }
  new_number->parts_capacity = parts_initial_capacity;
  new_number->is_negative = false;
  return new_number;
}

inline number *number_zero() {
  return number_from_int(0);
}

number *number_from_string(const char *string) {
  assert(string != NULL);
  char *string_copy = strdup(string);
  assert(string_copy != NULL);
  size_t string_length = strlen(string_copy),
      new_number_parts_size = (size_t) ceil((double) string_length / NUMBER_PART_SIZE);
  number *new_number = number_new(new_number_parts_size);
  new_number->is_negative = string_copy[0] == '-';
  for (int i = (int) string_length;
       i > (new_number->is_negative ? 1 : 0);
       i -= NUMBER_PART_SIZE
      ) {
    string_copy[i] = '\0';
    new_number->parts[new_number->parts_size++] = (int) strtol(
        i >= NUMBER_PART_SIZE
        ? string_copy + i - NUMBER_PART_SIZE
        : string_copy + (new_number->is_negative ? 1 : 0),
        NULL,
        10
    );
  }
  free(string_copy);
  return new_number;
}

number *number_from_int(int value) {
  number *new_number = number_new(1);
  new_number->parts[new_number->parts_size++] = abs(value);
  new_number->is_negative = value < 0;
  return new_number;
}

number *number_from_number(const number *source) {
  number *new_number = number_new(source->parts_capacity);
  new_number->parts_size = source->parts_size;
  new_number->is_negative = source->is_negative;
  memcpy(new_number->parts, source->parts, source->parts_size * sizeof(int));
  return new_number;
}

void number_append_part(number *number, int part) {
  assert(number != NULL);
  if (number->parts_size + 1 >= number->parts_capacity)
    number_parts_grow(number);
  number->parts[number->parts_size++] = part;
}

void number_parts_grow(number *number) {
  assert(number != NULL);
  size_t new_capacity = number->parts_capacity == 0
                        ? NUMBER_START_PARTS_CAPACITY
                        : number->parts_capacity * NUMBER_PARTS_CAPACITY_MULTIPLIER;
  return number_parts_grow_to(number, new_capacity);
}

void number_parts_grow_to(number *number, size_t new_capacity) {
  assert(number != NULL && new_capacity > 0);
  int *new_parts = realloc(number->parts, sizeof(int) * new_capacity);
  assert(new_parts != NULL);
  number->parts = new_parts;
  number->parts_capacity = new_capacity;
}

void number_remove_leading_zeroes(number *number) {
  while (number->parts_size > 1 && number->parts[number->parts_size - 1] == 0)
    number->parts_size--;
}

void number_sprint(const number *number, string *destination) {
  assert(number != NULL);
  string_grow_to(destination, number->parts_size * NUMBER_PART_SIZE + 2);
  if (number->is_negative)
    string_add(destination, '-');
  char buffer[NUMBER_PART_SIZE + 1];
  sprintf(buffer, "%d", number->parts_size == 0 ? 0 : number->parts[number->parts_size - 1]);
  string_append(destination, buffer);
  for (int i = (int) number->parts_size - 2; i >= 0; --i) {
    sprintf(buffer, NUMBER_PART_FORMAT, number->parts[i]);
    string_append(destination, buffer);
  }
}

void number_free(number *number) {
  assert(number != NULL);
  free(number->parts);
  free(number);
}

bool is_numbers_equal(const number *first, const number *second) {
  assert(first != NULL && second != NULL);
  if (first->parts_size != second->parts_size) return false;
  if (first->is_negative != second->is_negative) return false;
  for (size_t i = 0; i < first->parts_size; ++i)
    if (first->parts[i] != second->parts[i])
      return false;
  return true;
}

bool is_numbers_less(const number *first, const number *second) {
  assert(first != NULL && second != NULL);
  if (first->is_negative != second->is_negative)
    return first->is_negative;
  bool is_abs_less = is_numbers_abs_less(first, second);
  return (first->is_negative && second->is_negative) == !is_abs_less;
}

bool is_numbers_abs_less(const number *first, const number *second) {
  assert(first != NULL && second != NULL);
  if (first->parts_size != second->parts_size)
    return first->parts_size < second->parts_size;
  for (int i = (int) first->parts_size - 1; i >= 0; --i)
    if (first->parts[i] != second->parts[i])
      return first->parts[i] < second->parts[i];
  return false;
}

number *number_add(number *first, number *second) {
  assert(first != NULL && second != NULL);

  // a > 0, b < 0 => a+(-b) = a-b
  if (!first->is_negative && second->is_negative) {
    second->is_negative = false;
    return number_subtract(first, second);
  }

  // a < 0, b > 0 => (-a)+b = b-a
  if (first->is_negative && !second->is_negative) {
    first->is_negative = false;
    return number_subtract(second, first);
  }

  // a, b < 0 => (-a)+(-b) = -(a+b)
  if (first->is_negative && second->is_negative) {
    first->is_negative = second->is_negative = false;
    number *result = number_add(first, second);
    result->is_negative = true;
    return result;
  }

  bool carry = 0;
  for (size_t i = 0; i < max(first->parts_size, second->parts_size) || carry; ++i) {
    if (i == first->parts_size)
      number_append_part(first, 0);
    first->parts[i] += carry + (i < second->parts_size ? second->parts[i] : 0);
    carry = first->parts[i] >= NUMBER_BASE;
    if (carry)
      first->parts[i] -= NUMBER_BASE;
  }
  return number_from_number(first);
}

number *number_subtract(number *first, number *second) {
  assert(first != NULL && second != NULL);

  // b < 0 => a-(-b) = a+b
  if (second->is_negative) {
    second->is_negative = false;
    return number_add(first, second);
  }

  // a < 0 => (-a)-b = -(a+b)
  if (first->is_negative) {
    first->is_negative = false;
    number *result = number_add(first, second);
    result->is_negative = !result->is_negative;
    return result;
  }

  // a = b => a, b > 0
  if (is_numbers_equal(first, second))
    return number_zero();

  // a, b > 0, a < b => a-b = -(b-a)
  if (is_numbers_less(first, second)) {
    number *result = number_subtract(second, first);
    result->is_negative = true;
    return result;
  }

  bool carry = 0;
  for (size_t i = 0; i < second->parts_size || carry; ++i) {
    first->parts[i] -= carry + (i < second->parts_size ? second->parts[i] : 0);
    carry = first->parts[i] < 0;
    if (carry)
      first->parts[i] += NUMBER_BASE;
  }
  number_remove_leading_zeroes(first);
  return number_from_number(first);
}

number *number_multiply(number *first, number *second) {
  assert(first != NULL && second != NULL);
  size_t result_size = first->parts_size + second->parts_size;
  number *result = number_new(result_size);
  result->parts_size = result_size;
  result->is_negative = first->is_negative ^ second->is_negative;
  memset(result->parts, 0, sizeof(int) * result->parts_size);
  for (size_t i = 0; i < first->parts_size; ++i)
    for (int j = 0, carry = 0; j < (int) second->parts_size || carry; ++j) {
      long long current = result->parts[i + j] + first->parts[i] * 1ll *
          (j < (int) second->parts_size ? second->parts[j] : 0) + carry;
      result->parts[i + j] = (int) (current % NUMBER_BASE);
      carry = (int) (current / NUMBER_BASE);
    }
  number_remove_leading_zeroes(result);
  return result;
}

number *number_divide(number *first, number *second) {
  assert(first != NULL && second != NULL);

  if (is_numbers_abs_less(first, second))
    return number_zero();

  size_t result_size = first->parts_size;
  number *result = number_new(result_size);
  result->parts_size = result_size;
  result->is_negative = first->is_negative ^ second->is_negative;
  first->is_negative = second->is_negative = false;

  number *current = number_new(result_size);
  // Initialize as zero
  current->parts_size = 1;
  memset(current->parts, 0, sizeof(int) * current->parts_capacity);

  for (int i = (int) first->parts_size - 1; i >= 0; --i) {
    // current = current * NUMBER_BASE
    if (current->parts_size != 1 || current->parts[0] != 0) {
      for (size_t j = current->parts_size; j > 0; --j)
        current->parts[j] = current->parts[j - 1];
      current->parts_size++;
    }
    current->parts[0] = first->parts[i];
    // find max(x), second * x <= current
    int x = 0, left = 0, right = NUMBER_BASE;
    while (left <= right) {
      int medium = (left + right) / 2;
      number *second_multiply_medium = number_multiply(second, number_from_int(medium));
      if (is_numbers_less(second_multiply_medium, current) || is_numbers_equal(second_multiply_medium, current)) {
        x = medium;
        left = medium + 1;
      } else {
        right = medium - 1;
      }
      number_free(second_multiply_medium);
    }
    result->parts[i] = x;
    number *second_multiply_x = number_multiply(second, number_from_int(x));
    number_subtract(current, second_multiply_x);
    number_free(second_multiply_x);
  }

  number_remove_leading_zeroes(result);
  number_free(current);
  return result;
}

bool evaluate_expression(const string *expression, string *result) {
  assert(expression != NULL && result != NULL && !is_string_empty(expression));

  char_stack operators = STACK_INITIALIZER;
  number_stack operands = STACK_INITIALIZER;
  string number_string = STRING_INITIALIZER;
  char current = expression->content[0], previous = '\0';
  size_t i = 0;
  bool success = true;

  while (success && current != '\0' && current != '\n') {
    if (isdigit(current)) {
      string_add(&number_string, current);
    } else if (current == '-' && (previous == '\0' || is_operator(previous) || previous == '(')) {
      string_add(&number_string, '-');  // Add unary minus
    } else if (is_operator(current)) {
      // Push read number to stack
      if (!is_string_empty(&number_string)) {
        number_stack_push_string(&operands, number_string.content);
        string_clear(&number_string);
      }

      if (is_char_stack_empty(&operators) || char_stack_top(&operators) == '('
          || get_operator_precedence(current) > get_operator_precedence(char_stack_top(&operators))) {
        char_stack_push(&operators, current);
      } else {
        while (success && !is_char_stack_empty(&operators) && char_stack_top(&operators) != '('
            && get_operator_precedence(char_stack_top(&operators)) >= get_operator_precedence(current))
          success = calculate_expression_on_stack_top(&operators, &operands);
        if (!success) break;
        char_stack_push(&operators, current);
      }
    } else if (current == '(') {
      char_stack_push(&operators, current);
    } else if (current == ')') {
      // Push read number to stack
      if (!is_string_empty(&number_string)) {
        number_stack_push_string(&operands, number_string.content);
        string_clear(&number_string);
      }
      while (success && !is_char_stack_empty(&operators) && char_stack_top(&operators) != '(')
        success = calculate_expression_on_stack_top(&operators, &operands);
      if (!success) continue;
      if (is_char_stack_empty(&operators))
        success = false;
      else
        char_stack_pop(&operators);
    } else {
      success = false;
    }

    previous = current;
    current = expression->content[++i];
  }

  if (success) {
    if (!is_string_empty(&number_string))
      number_stack_push_string(&operands, number_string.content);
    while (success && !is_char_stack_empty(&operators))
      success = calculate_expression_on_stack_top(&operators, &operands);
    if (success) {
      number *result_number = number_stack_top(&operands);
      number_sprint(result_number, result);
    }
  }

  char_stack_free(&operators);
  number_stack_free(&operands);
  string_free(&number_string);
  return success;
}

inline void print_error() { printf("[error]"); }

inline bool is_operator(char c) { return c == '+' || c == '-' || c == '*' || c == '/'; }

size_t get_operator_precedence(char operator) {
  switch (operator) {
    case '+':
    case '-':return 1;
    case '*':
    case '/':return 2;
    default:return 0;
  }
}

bool calculate_expression_on_stack_top(char_stack *operators, number_stack *operands) {
  char operator = char_stack_top(operators);
  if (operator == '(') return false;
  number *first = number_stack_before_top(operands);
  number *second = number_stack_top(operands);

  number *result = calculate(first, second, operator);

  char_stack_pop(operators);
  number_stack_pop(operands);
  number_stack_pop(operands);
  number_stack_push(operands, result);
  number_free(result);
  return true;
}

number *calculate(number *first, number *second, char operator) {
  assert(first != NULL && second != NULL && is_operator(operator));
  if (operator == '+')
    return number_add(first, second);
  if (operator == '-')
    return number_subtract(first, second);
  if (operator == '*')
    return number_multiply(first, second);
  if (operator == '/')
    return number_divide(first, second);
  return NULL;
}

char *strdup(const char *string) {
  size_t length = strlen(string) + 1;
  char *new_string = malloc(sizeof(char) * length);
  assert(new_string != NULL);
  return memcpy(new_string, string, length);
}