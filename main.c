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
#define STR_MAX 45
#define SEPARATOR '-'

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
void centeredText(char *);
void printDashboard(player *, int, int);
int chooseRandom(int);
card cardConstructor(enum SUITS, enum TYPES);
void printPlayerDeck(player *);
void fillCardStack(stack *);
void shuffleCardStack(stack *);
bool buyCard(player *, stack *);

void addLog(char *);
void printLogs(void);
void clearLogs(void);

char *input(const char *);
int toint(const char *);
char *strstrip(char *);

static char **allocatedStrings = NULL;
static size_t allocatedStringsLength = 0;
static void teardownAllocations(void);

char **logs = NULL;
size_t logsLength = 0; 

int main(void) {
  atexit(teardownAllocations);

  int playersLength = 0;
  stack cardStack = {0};
  fillCardStack(&cardStack);

  while (true) {
    title();

    do {
      title();
      playersLength = toint(input("How many player? [2-8] "));
    }
    while(playersLength < 2 || playersLength > 8);

    shuffleCardStack(&cardStack);

    player players[playersLength];
    for (int c = 0; c < playersLength; c++) {
      do {
        title();
        printf("What's the %s player name? ", ordinal[c]);
        players[c].name = strstrip(input(""));
      }
      while (!*players[c].name);
      players[c].lenght = 0;
      players[c].deck = NULL;

      for (int k = 0; k < CARD_START; k++)
        buyCard(&players[c], &cardStack);
    }

    int currentPlayer = chooseRandom(playersLength);
    char formatedLog[STR_MAX];
    sprintf(formatedLog, "%s starts first!", players[currentPlayer].name);
    addLog(formatedLog);

    while (true) {
      for (int c = 0; c < playersLength; c++) {
        title();
        printLogs();
        printDashboard(players, currentPlayer, playersLength);
        printPlayerDeck(&players[currentPlayer]);

        system(PAUSE);
        currentPlayer = currentPlayer + 1 < playersLength ? currentPlayer + 1 : 0;
      }
      clearLogs();
      return 0;
    }
  }

  return 0;
}

void title(void) {
  system(CLEAR);
  printf("\n%s\n%s\n%s\n%s\n%s\n\n",
    "                   |                        ",
    ",---.,---.,---.,---|    ,---.,---.,-.-.,---.",
    "|    ,---||    |   |    |   |,---|| | ||---'",
    "`---'`---^`    `---'    `---|`---^` ' '`---'",
    "                        `---'               "
  );
}

void centeredText(char *text) {
  int length = strlen(text);
  int span = (STR_MAX - length) / 2;

  for (int c = 0; c < span; c++)
    printf("%c", SEPARATOR);

  for (int c = 0; text[c] != 0; c++)
    printf("%c", toupper(text[c]));

  if (length % 2 == 0)
    printf("%c", SEPARATOR);

  for (int c = 0; c < span; c++)
    printf("%c", SEPARATOR);
  puts("");
}

void printDashboard(player *players, int current, int lenght) {
  centeredText("players");
  for (int c = 0; c < lenght; c++) 
    printf("[%c][%02d] %-15s%c",
      current == c ? '*' : ' ',
      players[c].lenght,
      players[c].name,
      c % 2 ? '\n' : ' '
    );

  if (lenght % 2)
    puts("");
  puts("");
}

void addLog(char *text) {
  char **tmp = realloc(logs, logsLength + 1);
  if (tmp == NULL)
    return;
  logs = tmp;

  size_t length = strlen(text);
  char *newLog = malloc(sizeof(char) * length + 1);
  if (newLog == NULL)
    return;

  memcpy(newLog, text, length);
  newLog[length] = 0;

  logs[logsLength++] = newLog;
}

void printLogs(void) {
  centeredText("logs");
  if (logs == NULL)
    return;

  for (size_t c = 0; c < logsLength; c++)
    printf("* %-43s\n", logs[c]);
  puts("");
}

void clearLogs(void) {
  if (logs == NULL)
    return;

  for (size_t c = 0; c < logsLength; c++)
    free(logs[c]);

  free(logs);
  logs = NULL;
  logsLength = 0;
}

int chooseRandom(int lenght) {
  return rand() % lenght;
}

card cardConstructor(enum SUITS suit, enum TYPES type) {
  card newCard;
  newCard.suit = suit;
  newCard.type = type;
  return newCard;
}

void printPlayerDeck(player *current) {
  centeredText("deck");

  card *tmp = current->deck;
  for (int c = 0; c < current->lenght; c++, tmp = tmp->next)
    printf("[%02d] %5s of %-8s%c",
      c + 1,
      typeName[tmp->type],
      suitName[tmp->suit],
      c % 2 ? '\n' : ' '
    );

  if (current->lenght % 2)
    puts("");
  puts("");
}

void fillCardStack(stack *cardStack) {
  for (int suit = 0, i = 0; suit < SUIT_MAX; suit++)
    for (int type = 2; type < TYPE_MAX + 2; type++, i++)
      cardStack->cards[i] = cardConstructor(suit, type);

  cardStack->lenght = CARD_MAX;
}

void shuffleCardStack(stack *cardStack) {
  for (int i = 0; i < CARD_MAX; i++) {
    int j = i + rand() / (RAND_MAX / (CARD_MAX - i) + 1);
    card dummy = cardStack->cards[j];
    cardStack->cards[j] = cardStack->cards[i];
    cardStack->cards[i] = dummy;
  }
}

bool buyCard(player *current, stack* cardStack) {
  if (cardStack->lenght == 0)
    return false;

  card *newCard = (card *) malloc(sizeof(card));
  if (newCard == NULL)
    return false;

  cardStack->lenght--;
  newCard->suit = cardStack->cards[cardStack->lenght].suit;
  newCard->type = cardStack->cards[cardStack->lenght].type;

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
