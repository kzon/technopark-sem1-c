/*
  Кобзев Антон АПО-11

  Задача №1

  Реализуйте функцию
  char** div_format(char **s) {
      ...
  }
  и программу, использующую эту функцию для форматирования текстовой разметки состоящей из <div>, </div> и ASCII строк.
  Правильное форматирование подразумевает расстановку 4х пробелов согласно вложенности. То есть в исходном тексте
  необходимо игнорировать все "ведущие" и "закрывающие" пробелы, и форматировать вывод строк простановкой нового
  соответствующего количества пробелов. В случае некорректной последовательности тэгов, выведите в поток вывода [error].
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define TAB_SIZE 4

#define OPEN_TAG "<div>"
#define CLOSE_TAG "</div>"

#define START_STRING_SIZE 100
#define START_STRINGS_COUNT 10

#define MAX_TOKENS_COUNT 100
#define TOKEN_CONTENT_SIZE 1000

typedef enum { TOKEN_OPEN_TAG, TOKEN_CLOSE_TAG, TOKEN_TEXT } token_type;

typedef struct {
  token_type type;
  char content[TOKEN_CONTENT_SIZE];
} token;

typedef struct {
  char **strings;
  size_t size;
  size_t capacity;
} strings;

#define STRINGS_INITIALIZER {NULL, 0, 0}

void print_error();

bool insert_string(strings *arr, char *string);

void free_strings(strings *arr);

char *read_string();

bool read_strings(strings *input);

void print_strings(const strings *output);

char *trim_spaces(char *s);

size_t get_next_token(const char *s, token *token);

size_t tokenize(const strings *arr, token *tokens);

bool div_format(const strings *input, strings *output);

int main() {
  strings input = STRINGS_INITIALIZER;
  if (!read_strings(&input)) {
    print_error();
    free_strings(&input);
    return 0;
  }
  strings output = STRINGS_INITIALIZER;
  bool is_format_success = div_format(&input, &output);
  free_strings(&input);
  if (!is_format_success) {
    print_error();
    free_strings(&output);
    return 0;
  }
  print_strings(&output);
  free_strings(&output);
  return 0;
}

inline void print_error() { printf("[error]"); }

/* Returns true on success */
bool insert_string(strings *arr, char *string) {
  if (arr == NULL || string == NULL) return false;
  if (arr->strings == NULL) arr->size = arr->capacity = 0;
  if (arr->size + 1 >= arr->capacity) {
    size_t new_capacity = arr->capacity == 0 ? START_STRINGS_COUNT : arr->capacity * 2;
    char **new_strings = realloc(arr->strings, sizeof(char *) * new_capacity);
    if (new_strings == NULL) return false;
    arr->strings = new_strings;
    arr->capacity = new_capacity;
  }
  arr->strings[arr->size++] = string;
  return true;
}

void free_strings(strings *arr) {
  if (arr == NULL || arr->strings == NULL) return;
  for (size_t i = 0; i < arr->size; ++i)
    if (arr->strings[i] != NULL)
      free(arr->strings[i]);
  free(arr->strings);
  arr->strings = NULL;
  arr->size = arr->capacity = 0;
}

char *read_string() {
  struct {
    char *string;
    size_t size;
    size_t capacity;
  } buffer = {NULL, 0, 0};
  char c;
  while (c = (char) getchar(), c != EOF) {
    if (buffer.size + 1 >= buffer.capacity) {
      size_t new_capacity = buffer.capacity == 0 ? START_STRING_SIZE : buffer.capacity * 2;
      char *new_string = realloc(buffer.string, (new_capacity + 1) * sizeof(char));
      if (new_string == NULL) {
        if (buffer.string != NULL)
          free(buffer.string);
        return NULL;
      }
      buffer.string = new_string;
      buffer.capacity = new_capacity;
    }
    buffer.string[buffer.size] = c;
    buffer.string[++buffer.size] = '\0';
    if (c == '\n') break;
  }
  return buffer.string;
}

/* Returns true on success */
bool read_strings(strings *input) {
  if (input == NULL) return false;
  char *string;
  while ((string = read_string()) != NULL)
    if (!insert_string(input, string))
      return false;
  return true;
}

void print_strings(const strings *output) {
  if (output == NULL) return;
  for (size_t i = 0; i < output->size; ++i) {
    if (output->strings[i] == NULL) return;
    if (*output->strings[i] != '\0')
      printf("%s", output->strings[i]);
  }
}

char *trim_spaces(char *s) {
  if (s == NULL) return NULL;
  while (isspace(*s))
    s++;
  if (*s == '\0')
    return s;
  char *end = s + strlen(s) - 1;
  while (end > s && isspace(*end))
    end--;
  end[1] = '\0';
  return s;
}

/* Returns found token length */
size_t get_next_token(const char *s, token *const token) {
  if (s == NULL || token == NULL) return 0;
  char content[TOKEN_CONTENT_SIZE];
  size_t content_pos = 0, token_length = 0;
  while (*s == '\n') {
    s++;
    token_length++;
  }
  while (*s != '\0' && *s != '\n') {
    if (*s == '<') {
      if (content_pos != 0)
        break;
      if (*(s + 1) == '/') {
        token->type = TOKEN_CLOSE_TAG;
        strncpy(token->content, CLOSE_TAG, TOKEN_CONTENT_SIZE);
        return token_length + strlen(CLOSE_TAG);
      }
      if (*(s + 1) == 'd') {
        token->type = TOKEN_OPEN_TAG;
        strncpy(token->content, OPEN_TAG, TOKEN_CONTENT_SIZE);
        return token_length + strlen(OPEN_TAG);
      }
    }
    content[content_pos++] = *(s++);
  }
  content[content_pos] = '\0';
  token->type = TOKEN_TEXT;
  strncpy(token->content, trim_spaces(content), TOKEN_CONTENT_SIZE);
  return token_length + content_pos;
}

/* Returns number of found tokens */
size_t tokenize(const strings *arr, token *tokens) {
  if (arr == NULL || tokens == NULL) return 0;
  size_t token_length, tokens_count = 0, j;
  for (size_t i = 0; i < arr->size; ++i) {
    j = 0;
    while ((token_length = get_next_token(arr->strings[i] + j, &tokens[tokens_count])) != 0) {
      j += token_length;
      tokens_count++;
    }
  }
  return tokens_count;
}

/* Returns true on success */
bool div_format(const strings *input, strings *output) {
  if (input == NULL || output == NULL) return false;
  token tokens[MAX_TOKENS_COUNT];
  size_t tokens_count = tokenize(input, tokens), depth = 0;
  char *string;
  for (size_t i = 0; i < tokens_count; ++i) {
    if (tokens[i].type == TOKEN_CLOSE_TAG) {
      if (depth == 0) return false;
      depth -= TAB_SIZE;
    }

    if (*tokens[i].content != '\0') {
      string = malloc((depth + strlen(tokens[i].content) + 2) * sizeof(char));
      for (size_t j = 0; j < depth; ++j)
        string[j] = ' ';
      string[depth] = '\0';
      strcat(string, tokens[i].content);
      strcat(string, "\n");
      if (!insert_string(output, string)) return false;
    }

    if (tokens[i].type == TOKEN_OPEN_TAG)
      depth += TAB_SIZE;
  }
  return depth == 0;
}