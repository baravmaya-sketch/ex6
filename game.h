#define _CRT_SECURE_NO_WARNINGS
#ifndef GAME_H
#define GAME_H


#include "bst.h"

typedef enum { ARMOR, SWORD } ItemType;
typedef enum { PHANTOM, SPIDER, DEMON, GOLEM, COBRA } MonsterType;
typedef enum { UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3 } Direction;

typedef struct Item {
    char* name;
    ItemType type;
    int value;
} Item;

typedef struct Monster {
    char* name;
    MonsterType type;
    int hp;
    int maxHp;
    int attack;
} Monster;

typedef struct Room {
    int id;
    int x, y;
    int visited;
    Monster* monster;
    Item* item;
    struct Room* next;
} Room;

typedef struct Player {
    int hp;
    int maxHp;
    int baseAttack;
    BST* bag;
    BST* defeatedMonsters;
    Room* currentRoom;
} Player;

typedef struct {
    Room* rooms;
    Player* player;
    int roomCount;
    int configMaxHp;
    int configBaseAttack;
} GameState;

// Monster functions
void freeMonster(void* data);
int compareMonsters(void* a, void* b);
void printMonster(void* data);

// Item functions
void freeItem(void* data);
int compareItems(void* a, void* b);
void printItem(void* data);

// Game functions
void addRoom(GameState* g);
void initPlayer(GameState* g);
void playGame(GameState* g);
void freeGame(GameState* g);

//helper function
void printLegend(Room* rooms);
void addMonsterFunc(Room* room, GameState* g);
char* getItemTypeString(ItemType type);
char* getMonsterTypeString(MonsterType monType);
void addItemFunc(Room* room, GameState* g);
Room* findRoomByCoords(Room* head, int x, int y);
void printGameOptions();
void displayRoomAndPlayerStatus(GameState* g);
char* stringChooseDirection();

//free functions
static void freeRoom(Room* room);
static void freePlayer(Player* player);
static void freeGameState(GameState* game);
#endif
