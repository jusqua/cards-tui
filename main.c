#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#ifdef _WIN32
#define PAUSE "pause"
#define CLEAR "cls"
#else
#define PAUSE "read -p \"Press ENTER to continue...\""
#define CLEAR "clear"
#endif

#define CARD_START 4
#define SUIT_MAX 4
#define TYPE_MAX 13
#define CARD_MAX SUIT_MAX * TYPE_MAX

enum SUITS { HEART, SPADE, DIAMOND, CLUB };
enum TYPES { TWO = 2, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE };

typedef struct card {
  int suit;
  int type;
  struct card *next;
  struct card *prev;
}
card;

typedef struct player {
  char *name;
  card *deck;
  int lenght;
}
player;

typedef struct stack {
  card cards[CARD_MAX];
  int lenght;
}
stack;

const char *ordinal[] = {
  "first",
  "second",
  "third",
  "fourth",
  "fifth",
  "sixth",
  "seventh",
  "eighth"
};

const char *typeName[] = {
  "",
  "",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  "10",
  "jack",
  "queen",
  "king",
  "ace"
};

const char *suitName[] = {
  "hearts",
  "spades",
  "diamonds",
  "clubs"
};

void title(void);
void printDashboard(player *, int);
card cardConstructor(enum SUITS, enum TYPES);
void fillCardStack(void);
void shuffleCardStack(void);
bool buyCard(player *);

char *input(const char *);
int toint(const char *);
char *strstrip(char *);

int playersLength;
stack cardStack = {0};

static char **allocatedStrings = NULL;
static size_t allocatedStringsLength = 0;
static void teardownAllocations(void);

int main(void) {
  atexit(teardownAllocations);

  while (true) {
    title();

    do {
      title();
      playersLength = toint(input("How many player? [2-8] "));
    }
    while(playersLength < 2 || playersLength > 8);

    fillCardStack();
    shuffleCardStack();

    player players[playersLength];
    for (int c = 0; c < playersLength; c++) {
      do {
        title();
        printf("What's the %s player name? ", ordinal[c]);
        players[c].name = strstrip(input(""));
      }
      while (!*players[c].name);
      players[c].lenght = 0;
      for (int k = 0; k < CARD_START; k++)
        buyCard(&players[c]);
    }

    title();

    for (int c = 0; c < CARD_MAX; c++)
      printf("%s of %s\n", typeName[cardStack.cards[c].type], suitName[cardStack.cards[c].suit]);

    break;
  }

  return 0;
}

void title(void) {
  system(CLEAR);
  puts("~ ~ ~ CARD GAME ~ ~ ~\n\n");
}

void printDashboard(player *players, int current) {
  for (int c = 0; c < playersLength; c++)
    printf("%c [%d] %-20s\n", current == c ? '*' : ' ', players[c].lenght, players[c].name);
}

card cardConstructor(enum SUITS suit, enum TYPES type) {
  card newCard;
  newCard.suit = suit;
  newCard.type = type;
  return newCard;
}

void fillCardStack(void) {
  for (int suit = 0, i = 0; suit < SUIT_MAX; suit++)
    for (int type = 2; type < TYPE_MAX + 2; type++, i++)
      cardStack.cards[i] = cardConstructor(suit, type);

  cardStack.lenght = CARD_MAX;
}

void shuffleCardStack(void) {
  for (int i = 0; i < CARD_MAX; i++) {
    int j = i + rand() / (RAND_MAX / (CARD_MAX - i) + 1);
    card dummy = cardStack.cards[j];
    cardStack.cards[j] = cardStack.cards[i];
    cardStack.cards[i] = dummy;
  }
}

bool buyCard(player *current) {
  card *newCard = (card *) malloc(sizeof(card));
  if (cardStack.lenght == 0)
    return false;

  cardStack.lenght--;
  newCard = &cardStack.cards[cardStack.lenght];
  if (current->deck == NULL) {
    current->deck = newCard;
    newCard->next = newCard;
    newCard->prev = newCard;
  }
  else {
    newCard->next = current->deck;
    newCard->prev = current->deck->prev;
    current->deck->prev = newCard;
    newCard->prev->next = newCard;
  }
  current->lenght++;

  return true;
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

int toint(const char *string) {
  if (string != NULL && *string && !isspace((unsigned char) *string)) {
    errno = 0;
    char *endptr;
    long num = strtol(string, &endptr, 10);
    if (errno == 0 && !(*endptr) && num >= INT_MIN && num < INT_MAX)
      return num;
  }

  return INT_MAX;
}

char *strstrip(char *string) {
  if (string == NULL)
    return NULL;

  size_t size = strlen(string);
  if (!size)
    return string;

  char *end = string + size - 1;
  while (end >= string && isspace((unsigned char) *end))
    end--;
  *(end + 1) = '\0';

  while (*string && isspace((unsigned char) *string))
    string++;

  return string;
}
