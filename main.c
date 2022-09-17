#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#define PAUSE "pause"
#define CLEAR "cls"
#else
#define PAUSE "read -p \"Press ENTER to continue...\""
#define CLEAR "clear"
#endif

char *input(const char *);

static char **allocatedStrings = NULL;
static size_t allocatedStringsLength = 0;
static void teardownAllocations(void);

int main(void) {
  atexit(teardownAllocations);

  return 0;
}

// simplified libcs50 get_string: https://github.com/cs50/libcs50
char *input(const char *format) {
  if (allocatedStringsLength == SIZE_MAX / sizeof (char *))
    return NULL;

  int ch;
  char *string = NULL;
  size_t stringLength = 0;

  printf("%s", format);

  while ((ch = fgetc(stdin)) != '\r' && ch != '\n' && ch != EOF) {
    if (stringLength == SIZE_MAX) {
      free(string);
      return NULL;
    }

    char *tmpString = realloc(string, stringLength + 1);
    if (tmpString == NULL) {
      free(string);
      return NULL;
    }
    string = tmpString;

    string[stringLength++] = ch;
    tmpString = NULL;
  }

  if (stringLength == 0 && ch == EOF)
    return NULL;

  if (ch == '\r' && (ch = fgetc(stdin)) != '\n') {
    if (ch != EOF && ungetc(ch, stdin) == EOF) {
      free(string);
      return NULL;
    }
  }

  char *tmpString = realloc(string, stringLength + 1);
  if (tmpString == NULL) {
    free(string);
    return NULL;
  }
  string = tmpString;

  string[stringLength] = '\0';

  char **tmpAllocatedStrings = realloc(allocatedStrings, sizeof (char *) * (allocatedStringsLength + 1));
  if (tmpAllocatedStrings == NULL) {
    free(string);
    return NULL;
  }
  allocatedStrings = tmpAllocatedStrings;

  allocatedStrings[allocatedStringsLength++] = string;

  return string;
}

static void teardownAllocations(void) {
  if (allocatedStrings != NULL) {
    for (size_t c = 0; c < allocatedStringsLength; c++)
      free(allocatedStrings[c]);
    free(allocatedStrings);
  }
}

