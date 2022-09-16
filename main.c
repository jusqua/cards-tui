#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <float.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#ifdef _WIN32
#define PAUSE "pause"
#define CLEAR "cls"
#else
#define PAUSE "read -p \"Press ENTER to continue...\""
#define CLEAR "clear"
#endif

char *get_string(const char *);
int get_int(const char *);
char get_char(const char *);
float get_float(const char *);

static void teardown(void);
static size_t allocations = 0;
static char **strings = NULL;

int main(void) {
  atexit(teardown);

  puts("Hello, World!");
  return 0;
}

// based on libcs50: https://github.com/cs50/libcs50
char *get_string(const char *format) {
  int c;
  char *string, *buffer = NULL;
  size_t size = 0;

  if (allocations == SIZE_MAX / sizeof(char *))
    return NULL;

  printf("%s", format);

  while ((c = fgetc(stdin)) != '\r' && c != '\n' && c != EOF) {
    if (size >= SIZE_MAX) {
      free(buffer);
      return NULL;
    }

    buffer = realloc(buffer, size + 1);
    if (buffer == NULL) {
      free(buffer);
      return NULL;
    }
  buffer[size++] = c;
  }

  if (size == 0 && c == EOF) {
    return NULL;
  }

  if (size == SIZE_MAX) {
    free(buffer);
    return NULL;
  }

  if (c == '\r' && (c = fgetc(stdin)) == '\n') {
    if (c != EOF && ungetc(c, stdin) == EOF) {
      free(buffer);
      return NULL;
    }
  }

  string = realloc(buffer, size + 1);
  if (string == NULL) {
    free(string);
    return NULL;
  }
  string[size] = '\0';

  strings = realloc(strings, sizeof(char *) * (allocations + 1));
  if (strings == NULL) {
    free(string);
    return NULL;
  }

  strings[allocations++] = string;

  return string;
}

int get_int(const char *format) {
  char *line, *tail;
  long num;
  errno = 0;

  while (true) {
    line = get_string(format);
    if (line == NULL)
      return INT_MAX;

    if (*line && !isspace((unsigned char) *line)) {
      num = strtol(line, &tail, 10);
      if (errno == 0 && !(*tail) && num >= INT_MIN && num < INT_MAX)
        return num;
    }
  }
}

char get_char(const char *format) {
  char *line, c, d;

  while (true) {
    line = get_string(format);
    if (line == NULL)
      return CHAR_MAX;

    if (sscanf(line, "%c%c", &c, &d) <= 1)
      return c;
  }
}

float get_float(const char *format) {
  char *line, *tail;
  float num;
  errno = 0;

  while (true) {
    line = get_string(format);
    if (line == NULL)
      return FLT_MAX;

    if (*line && !isspace((unsigned char) *line)) {
      num = strtof(line, &tail);
      if (errno == 0 && !(*tail) && isfinite(num) && num < FLT_MAX)
        if (strcspn(line, "XxEePp") == strlen(line))
          return num;
    }
  }
}

static void teardown(void) {
  if (strings != NULL) {
    for (size_t i = 0; i < allocations; i++)
      free(strings[i]);
    free(strings);
  }
}
