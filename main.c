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
bool hasCardSuitable(int, player *);
bool isCardSuitable(int, int, player *);
card discardCard(int, player *);
card cardConstructor(int, enum TYPES);
void fillCardStack();
void shuffleCardStack();
bool buyCard(player *);
int calculateEmptyStackWinner(player *);
int calculateRoundWinner(int *);
void addLog(char *);
void printLogs(void);
void clearLogs(void);
void clearDeck(player *);
int buyUntilFindCardSuitable(int, player *);
void game(void);
void rules(void);

char *input(const char *);
int toint(const char *);

static char **allocatedStrings = NULL;
static size_t allocatedStringsLength = 0;
static void teardownAllocations(void);

static char formatedLog[STR_MAX] = "";
static char **logs = NULL;
static size_t logsLength = 0; 

stack cardStack = {0};
int playersLength = 0;

int main(void) {
  atexit(teardownAllocations);
  fillCardStack();
  bool loopRunning = true;

  while (loopRunning) {
    title();
    puts("[P]lay the game");
    puts("[S]ee the rules");
    puts("[E]xit game");
    puts("");

    char *option = input("Which option? ");
    if (option[0] == 0 || option[1] != 0)
      continue;

    switch (toupper(option[0])) {
      case 'P':
        do game();
        while (toupper(input("Play again? [N/y]")[0]) == 'Y');
        break;
      case 'S':
        rules();
        break;
      case 'E':
        loopRunning = false;
        break;
    }
  }

  return 0;
}

void game(void) {
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
      players[c].name = input("");
    }
    while (!*players[c].name);
    players[c].lenght = 0;
    players[c].deck = NULL;

    for (int k = 0; k < CARD_START; k++)
      buyCard(&players[c]);
  }

  card currentCard;
  int firstCardSuit;
  int cardAdded;
  int currentPlayer;
  int cardValues[playersLength];

  int playerCard = 0;
  int currentRound = 0;
  bool isGameOver = false;

  while (!isGameOver) {
    if (currentRound != 0) {
      currentPlayer = calculateRoundWinner(cardValues);
      sprintf(formatedLog, "'%s' wins this round!", players[currentPlayer].name);
      addLog(formatedLog);

      title();
      printLogs();
      centeredText("round over");
      system(PAUSE);
    }

    firstCardSuit = -1;
    currentRound++;
    clearLogs();

    sprintf(formatedLog, "Round %d begins", currentRound);
    addLog(formatedLog);

    sprintf(formatedLog, "%d cards remains in stack to buy", cardStack.lenght);
    addLog(formatedLog);

    if (currentRound == 1) {
      currentPlayer = rand() % playersLength;
      sprintf(formatedLog, "'%s' has chosen randonly to starts first", players[currentPlayer].name);
    }
    else {
      sprintf(formatedLog, "'%s' starts first", players[currentPlayer].name);
    }
    addLog(formatedLog);

    for (int c = 0; c < playersLength; c++) {
      if (!hasCardSuitable(firstCardSuit, &players[currentPlayer])) {
        if ((cardAdded = buyUntilFindCardSuitable(firstCardSuit, &players[currentPlayer])) == -1) {
          isGameOver = true;

          int winner = calculateEmptyStackWinner(players);
          sprintf(formatedLog, "The stack is out, the game is over!");
          addLog(formatedLog);
          sprintf(formatedLog, "Calculating all cards from all players");
          addLog(formatedLog);
          sprintf(formatedLog, "'%s' wins by having the lowest deck", players[winner].name);
          addLog(formatedLog);

          break;
        }
        sprintf(formatedLog, "'%s' purchased %d new cards", players[currentPlayer].name, cardAdded);
        addLog(formatedLog);
      }

      do {
        title();
        printLogs();
        printDashboard(players, currentPlayer);
        playerCard = toint(input("Which card? "));
        playerCard--;
      }
      while (!isCardSuitable(playerCard, firstCardSuit, &players[currentPlayer]));

      currentCard = discardCard(playerCard, &players[currentPlayer]);
      if (c == 0)
        firstCardSuit = currentCard.suit;
      cardValues[currentPlayer] = currentCard.type;

      sprintf(formatedLog, "'%s' throws '%s of %s'",
        players[currentPlayer].name,
        typeName[currentCard.type],
        suitName[currentCard.suit]
      );
      addLog(formatedLog);

      if (!players[currentPlayer].lenght) {
        isGameOver = true;

        sprintf(formatedLog, "'%s' is out of cards, we found our winner!", players[currentPlayer].name);
        addLog(formatedLog);

        break;
      }

      currentPlayer = currentPlayer + 1 < playersLength ? currentPlayer + 1 : 0;
    }
  }
  title();
  printLogs();
  centeredText("game over");
  clearLogs();
  clearDeck(players);
}

void rules(void) {
  title();
  centeredText("cards");
  puts("* In the game, each card has a suit and a type diferent");
  puts("* 4 suits and 13 types, add up to 52 card in a stack");
  puts("  - The suits are: hearts, spades, diamonds and clubs");
  puts("  - Every type has a value: numbers from 2 to 10, jack");
  puts("      queen, king and ace, smallest to highest respectively");
  puts("  - the ace type has power of 14 but value of 1");
  puts("");

  centeredText("how works");
  puts("* The game allows 2 up to 8 players");
  puts("* The purchase stack is shuffled");
  puts("* Each player buy 4 cards");
  puts("* Randonly a player is choosed to start");
  puts("* The discard occurs circuling the player list");
  puts("  - Ex: Player 8 starts, next is the player 1, next is 2, and so on");
  puts("* The player who starts can choose the suit he wants");
  puts("* The other players must discard only cards with the same suit");
  puts("* Who discard the most valuable card wins");
  puts("* In case of the player has no cards to discard, then");
  puts("    will purchase from stack until find one suitable");
  puts("* In case of the stack runs out, the winner of the game is");
  puts("    the player who has the lowest valueble deck");
  puts("  - Ex: ");
  puts("    player 1 has: 2 + 4 + 12 (queen) + 1 (ace) = 19");
  puts("    player 2 has: 10 + 9 + 13 (king) = 32");
  puts("    player 1 wins");
  puts("* The players whos discard his entire deck wins");
  puts("");

  centeredText("the interface");
  printf("\n\n%s\n%s\n%s\n%s\n%s\n\n",
    "                   |                         ",
    ",---.,---.,---.,---|     ,---.,---.,-.-.,---.",
    "|    ,---||    |   |     |   |,---|| | ||---'",
    "`---'`---^`    `---'     `---|`---^` ' '`---'",
    "                         `---'               "
  );
  centeredText("logs");
  printf("%s\n%s\n%s\n\n",
    "* Round 1 begins",
    "* 44 cards remains in stack to buy",
    "* '2' has chosen randonly to starts first <- Here is the round logs"
  );
  centeredText("players");
  puts("[ ][04] 1               [*][04] 2 <- current player has a '*'\n");
  centeredText("deck");
  printf("%s\n%s\n\n",
    "[01]  king of hearts   [02]     7 of hearts",
    "[03]     6 of hearts   [04]     2 of spades"
  );
  puts("Which card? <- The player choose his card from deck");
  puts("");

  system(PAUSE);
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
}

int buyUntilFindCardSuitable(int suit, player *current) {
  int numOfCardsAdded = 0;

  do {
    if (!buyCard(current))
      return -1;
    numOfCardsAdded++;
  }
  while (current->deck->prev->suit != suit);

  return numOfCardsAdded;
}

int calculateEmptyStackWinner(player *players) {
  int sum, winner, max = 0;
  card *tmp;
  for (int c = 0; c < playersLength; c++) {
    sum = 0;
    tmp = players[c].deck;

    for (int k = 0; k < players[c].lenght; k++, tmp = tmp->next)
      sum += tmp->suit == ACE ? 1 : tmp->suit;

    if (sum > max) {
      max = sum;
      winner = c;
    }
  }
  return winner;
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

bool hasCardSuitable(int suit, player *current) {
  if (suit == -1)
    return true;

  card *tmp = current->deck;
  for (int c = 0; c < current->lenght; c++) {
    tmp = tmp->next;
    if (tmp->suit == suit) 
      return true;
  }

  return false;
}

bool isCardSuitable(int playerCard, int suit, player *current) {
  if (playerCard < 0 || playerCard >= current->lenght) 
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

  if (tmp == current->deck)
    current->deck = current->deck->next;

  tmp->prev->next = tmp->next;
  tmp->next->prev = tmp->prev;
  free(tmp);
  current->lenght--;

  return currentCard;
}

void clearDeck(player *players) {
  for (int c = 0; c < playersLength; c++) {
    while (players[c].lenght != 0)
      discardCard(0, &players[c]);
    players[c].deck = NULL;
  }
}

void addLog(char *text) {
  char **tmp = (char **) realloc(logs, sizeof (char *) * (logsLength + 1));
  if (tmp == NULL) 
    return;
  logs = tmp;

  size_t length = strlen(text);
  char *newLog = (char *) malloc(sizeof(char) * (length + 1));
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


card cardConstructor(int suit, enum TYPES type) {
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
  int range = rand() % (rand() % 10);
  for (int c = 0; c < range; c++) {
    for (int i = 0; i < CARD_MAX; i++) {
      int j = i + rand() / (RAND_MAX / (CARD_MAX - i) + 1);
      card dummy = cardStack.cards[j];
      cardStack.cards[j] = cardStack.cards[i];
      cardStack.cards[i] = dummy;
    }
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
