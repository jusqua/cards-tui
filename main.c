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
void printDashboard(player *, int);
bool isCardSuitable(int, enum SUITS, player *);
card discardCard(int, player *);
int chooseRandom();
card cardConstructor(enum SUITS, enum TYPES);
void fillCardStack();
void shuffleCardStack();
bool buyCard(player *);
int calculateRoundWinner(int *);

void addLog(char *);
void clearLogs(void);

char *input(const char *);
int toint(const char *);
char *strstrip(char *);

static char **allocatedStrings = NULL;
static size_t allocatedStringsLength = 0;
static void teardownAllocations(void);

stack cardStack = {0};
char **logs = NULL;
int playersLength = 0;
size_t logsLength = 0; 

int main(void) {
  atexit(teardownAllocations);

  fillCardStack();

  while (true) {
    title();

    do {
      title();
      playersLength = toint(input("How many player? [2-8] "));
    }
    while(playersLength < 2 || playersLength > 8);

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
      players[c].deck = NULL;

      for (int k = 0; k < CARD_START; k++)
        buyCard(&players[c]);
    }

    int currentPlayer = chooseRandom(playersLength);
    char formatedLog[STR_MAX] = "";
    int cardValues[playersLength];
    card currentCard;
    int playerCard = 0;
    int firstCardSuit = -1;

    sprintf(formatedLog, "'%s' starts first!", players[currentPlayer].name);
    addLog(formatedLog);

    while (true) {
      for (int c = 0; c < playersLength; c++) {
        do {
          title();
          printDashboard(players, currentPlayer);
          playerCard = toint(input("Which card? "));
        }
        while (isCardSuitable(playerCard, firstCardSuit, &players[currentPlayer]));

        currentCard = discardCard(playerCard, &players[currentPlayer]);
        if (c == 0)
          firstCardSuit = currentCard.suit;
        cardValues[currentPlayer] = currentCard.type;

        currentPlayer = currentPlayer + 1 < playersLength ? currentPlayer + 1 : 0;
      }
      firstCardSuit = -1;
      clearLogs();

      currentPlayer = calculateRoundWinner(cardValues);

      return 0;
    }
  }

  return 0;
}

void title(void) {
  system(CLEAR);
  printf("\n%s\n%s\n%s\n%s\n%s\n\n",
    "                   |                         ",
    ",---.,---.,---.,---|     ,---.,---.,-.-.,---.",
    "|    ,---||    |   |     |   |,---|| | ||---'",
    "`---'`---^`    `---'     `---|`---^` ' '`---'",
    "                         `---'               "
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

void printDashboard(player *players, int current) {
  centeredText("players");
  for (int c = 0; c < playersLength; c++) 
    printf("[%c][%02d] %-15s%c",
      current == c ? '*' : ' ',
      players[c].lenght,
      players[c].name,
      c % 2 ? '\n' : ' '
    );

  if (playersLength % 2)
    puts("");
  puts("");

  centeredText("deck");
  card *tmp = players[current].deck;
  for (int c = 0; c < players[current].lenght; c++, tmp = tmp->next)
    printf("[%02d] %5s of %-8s%c",
      c + 1,
      typeName[tmp->type],
      suitName[tmp->suit],
      c % 2 ? '\n' : ' '
    );
  if (players[current].lenght % 2)
    puts("");
  puts("");

  centeredText("logs");
  if (logs == NULL)
    return;
  for (size_t c = 0; c < logsLength; c++)
    printf("* %-43s\n", logs[c]);
  puts("");
}

int calculateRoundWinner(int *values) {
  int maxValue = 0;
  int winner = -1;
  for (int c = 0; c < playersLength; c++) {
    if (values[c] > maxValue) {
      maxValue = values[c];
      winner = c;
    }
  }

  return winner;
}

bool isCardSuitable(int playerCard, enum SUITS suit, player *current) {
  if (playerCard < 1 || playerCard > current->lenght) 
    return false;

  if (suit == -1)
    return true;

  card *tmp = current->deck;
  for (int c = 0; c < playerCard; c++)
    tmp = tmp->next;

  if (tmp->suit == suit)
    return true;

  return false;
}

card discardCard(int playerCard, player *current) {
  card currentCard;

  card *tmp = current->deck;
  for (int c = 0; c < playerCard; c++)
    tmp = tmp->next;

  currentCard.suit = tmp->suit;
  currentCard.type = tmp->type;

  tmp->prev->next = tmp->next;
  tmp->next->prev = tmp->prev;
  free(tmp);

  return currentCard;
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

void clearLogs(void) {
  if (logs == NULL)
    return;

  for (size_t c = 0; c < logsLength; c++)
    free(logs[c]);

  free(logs);
  logs = NULL;
  logsLength = 0;
}

int chooseRandom() {
  return rand() % playersLength;
}

card cardConstructor(enum SUITS suit, enum TYPES type) {
  card newCard;
  newCard.suit = suit;
  newCard.type = type;
  return newCard;
}

void fillCardStack() {
  for (int suit = 0, i = 0; suit < SUIT_MAX; suit++)
    for (int type = 2; type < TYPE_MAX + 2; type++, i++)
      cardStack.cards[i] = cardConstructor(suit, type);

  cardStack.lenght = CARD_MAX;
}

void shuffleCardStack() {
  for (int i = 0; i < CARD_MAX; i++) {
    int j = i + rand() / (RAND_MAX / (CARD_MAX - i) + 1);
    card dummy = cardStack.cards[j];
    cardStack.cards[j] = cardStack.cards[i];
    cardStack.cards[i] = dummy;
  }
}

bool buyCard(player *current) {
  if (cardStack.lenght == 0)
    return false;

  card *newCard = (card *) malloc(sizeof(card));
  if (newCard == NULL)
    return false;

  cardStack.lenght--;
  newCard->suit = cardStack.cards[cardStack.lenght].suit;
  newCard->type = cardStack.cards[cardStack.lenght].type;

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
