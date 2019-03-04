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
#include <ctype.h>
#include <string.h>

#define TAB_SIZE 4

#define OPEN_TAG "<div>"
#define CLOSE_TAG "</div>"

#define MAX_LINE_SIZE 10000
#define MAX_LINES_COUNT 100
#define MAX_TOKENS_COUNT 100
#define TOKEN_CONTENT_SIZE 1000

typedef enum { TOKEN_OPEN_TAG, TOKEN_CLOSE_TAG, TOKEN_TEXT } token_type;

typedef struct {
  token_type type;
  char content[TOKEN_CONTENT_SIZE];
} token;

void print_error();

char **alloc_strings_array(size_t n, size_t string_size);

void free_strings_array(char **arr, size_t n);

void read_lines(char *const *input);

void print_lines(char *const *output);

char *trim_spaces(char *s);

size_t get_next_token(const char *s, token *token);

size_t tokenize(char **s, token *tokens);

static int div_format_error = 0;
char **div_format(char **s);

int main() {
  char **input = alloc_strings_array(MAX_LINES_COUNT, MAX_LINE_SIZE);
  if (input == NULL) {
    print_error();
    return 0;
  }
  read_lines(input);
  char **output = div_format(input);
  if (output == NULL) {
    print_error();
    return 0;
  }
  free_strings_array(input, MAX_LINES_COUNT);
  if (div_format_error)
    print_error();
  else
    print_lines(output);
  free_strings_array(output, MAX_LINES_COUNT);
  return 0;
}

inline void print_error() {
  printf("[error]");
}

/* Returns NULL on failure */
char **alloc_strings_array(size_t n, size_t string_size) {
  if (n == 0 || string_size == 0) return NULL;
  char **arr = malloc(n * sizeof(char *));
  if (arr == NULL) return NULL;
  for (size_t i = 0; i < n; ++i) {
    arr[i] = calloc(string_size, sizeof(char));
    if (arr[i] == NULL) return NULL;
  }
  return arr;
}

void free_strings_array(char **arr, size_t n) {
  if (arr == NULL) return;
  for (size_t i = 0; i < n; ++i)
    free(arr[i]);
  free(arr);
}

void read_lines(char *const *input) {
  if (input == NULL) return;
  char line[MAX_LINE_SIZE];
  size_t lines_read = 0;
  while (fgets(line, MAX_LINE_SIZE, stdin) != NULL)
    strncpy(input[lines_read++], line, MAX_LINE_SIZE);
}

void print_lines(char *const *output) {
  if (output == NULL) return;
  for (size_t i = 0; i < MAX_LINES_COUNT; ++i) {
    if (output[i] == NULL) return;
    if (*output[i] != '\0')
      printf("%s\n", output[i]);
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
size_t tokenize(char **s, token *tokens) {
  if (s == NULL || *s == NULL || tokens == NULL) return 0;
  size_t token_length, tokens_count = 0, j;
  for (size_t i = 0; i < MAX_LINES_COUNT; ++i) {
    j = 0;
    while ((token_length = get_next_token(s[i] + j, &tokens[tokens_count])) != 0) {
      j += token_length;
      tokens_count++;
    }
  }
  return tokens_count;
}

/* Returns new allocated strings array or NULL on failure */
char **div_format(char **s) {
  if (s == NULL || *s == NULL) return NULL;
  char **output = alloc_strings_array(MAX_LINES_COUNT, MAX_LINE_SIZE);
  if (output == NULL) return NULL;
  token tokens[MAX_TOKENS_COUNT];
  size_t tokens_count = tokenize(s, tokens), lines_count = 0, depth = 0;
  for (size_t i = 0; i < tokens_count; ++i) {
    if (tokens[i].type == TOKEN_CLOSE_TAG) {
      if (depth == 0) {
        div_format_error = 1;
        return output;
      }
      depth -= TAB_SIZE;
    }

    if (*tokens[i].content != '\0') {
      for (size_t j = 0; j < depth; ++j)
        output[lines_count][j] = ' ';
      strncpy(output[lines_count++] + depth, tokens[i].content, MAX_LINE_SIZE - depth);
    }

    if (tokens[i].type == TOKEN_OPEN_TAG)
      depth += TAB_SIZE;
  }
  if (depth != 0)
    div_format_error = 1;
  return output;
}