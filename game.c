#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "utils.h"

// Map display functions
static void displayMap(GameState* g) {
    if (!g->rooms) 
        return;
    
    // Find bounds
    int minX = 0, maxX = 0, minY = 0, maxY = 0;
    for (Room* r = g->rooms; r; r = r->next) {
        if (r->x < minX) minX = r->x;
        if (r->x > maxX) maxX = r->x;
        if (r->y < minY) minY = r->y;
        if (r->y > maxY) maxY = r->y;
    }
    
    int width = maxX - minX + 1;
    int height = maxY - minY + 1;
    
    // Create grid
    int** grid = malloc(height * sizeof(int*));
    for (int i = 0; i < height; i++) {
        grid[i] = malloc(width * sizeof(int));
        for (int j = 0; j < width; j++) grid[i][j] = -1;
    }
    
    for (Room* r = g->rooms; r; r = r->next)
        grid[r->y - minY][r->x - minX] = r->id;
    
    printf("=== SPATIAL MAP ===\n");
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (grid[i][j] != -1) printf("[%2d]", grid[i][j]);
            else printf("    ");
        }
        printf("\n");
    }
    
    for (int i = 0; i < height; i++) free(grid[i]);
    free(grid);
}

void addRoom(GameState* g) {
    printMap(g->rooms);
    int baseId = getInt("Attach to room ID");
    Room* baseRoom = findRoomById(g->rooms, baseId);


    int roomDirec = getInt("Direction (0=Up,1=Down,2=Left,3=Right):");


    int x = baseRoom->x;
    int y = baseRoom->y;
    computeNewCoords(roomDirec, &x, &y);

    if (isRoomOccupied(g->rooms, x, y)) {
        printf("Room exists there\n");
        return;
    }
    Room* newRoom = (Room*)malloc(sizeof(Room));
    if (newRoom == NULL)
        exit(1);

    newRoom->x = x;
    newRoom->y = y;
    newRoom->id = g->roomCount++;
    newRoom->visited = 0;
    newRoom->monster = NULL;
    newRoom->item = NULL;

    int addMonster = getInt("Add monster? (1=Yes, 0=No):");
    if (addMonster) {
        addMonsterFunc(newRoom);
    }

    int addItem = getInt("Add item? (1=Yes, 0=No):");
    if (addItem)
        addItemFunc(newRoom);

    print("Created room %d at (%d, %d)", newRoom->id, newRoom->x, newRoom->y);
}

// Helper function to allocate and initialize a monster in the room
void addMonsterFunc(Room* room) {
    // Allocate memory for the monster struct
    room->monster = (Monster*)malloc(sizeof(Monster));
    if (room->monster == NULL) {
        exit(1);
    }

    // Get monster details from user
    room->monster->name = getString("Monster name: ");
    room->monster->type = (MonsterType)getInt("Type (0-4): ");
    // Using getInt for safe integer input
    room->monster->hp = getInt("HP: ");
    room->monster->attack = getInt("Attack:");
}

// Helper function to allocate and initialize an item in the room
void addItemFunc(Room* room) {
    // Allocate memory for the item struct
    room->item = (Item*)malloc(sizeof(Item));
    if (room->item == NULL) {
        exit(1);
    }

    // Get item details from user
    room->item->name = getString("Item name: ");
    // Assuming ItemType is an enum (0-3), we cast the integer input
    room->item->type = (ItemType)getInt("Type (0=Armor, 1=Sword):");
    room->item->value = getInt("Value: ");
}

static void computeNewCoords(int roomDirec, int* x, int* y) {

    enum Direction direc = (enum Direction)roomDirec;


    switch (direc) {
    case UP:
        y--;
        break;
    case DOWN:
        y++;
        break;
    case RIGHT:
        x++;
        break;
    case LEFT:
        x--;
        break;
    }
}

//return 1 for occupied and 0 for free 
static int isRoomOccupied(Room* head, int x, int y) {
    Room* current = head;
    while (current != NULL) {
        if (current->x == x && current->y == y) {
            return 1; // Occupied
        }
        current = current->next;
    }
    return 0; // Free
}

//Find a room in the linked list by its unique ID
static Room* findRoomById(Room* head, int roomID) {
    Room* current = head;
    while (current != NULL) {
        if (current->id == roomID)
            return current;
        current = current->next;
    }
    return NULL;
}

/*
 * Initializes the player: allocates memory, sets initial stats (HP, Attack),
 * locates the starting room, and creates the backpack and monster log.
 */
void initPlayer(GameState* g) {
    //safety check
    if (g == NULL)
        return;

    g->player = malloc(sizeof(Player));
    if (g->player == NULL)
        exit(1);

    g->player->maxHp = g->configMaxHp;
    g->player->hp = g->configMaxHp;
    //initialize first room as current room
    g->player->currentRoom = findRoomById(g->rooms, 0);
    g->player->baseAttack = g->configBaseAttack;
    g->player->bag = createBST(compareItems, printItem, freeItem);
    g->player->defeatedMonsters = createBST(compareMonsters, printMonster, freeMonster);
}

/*
 * Compares two items based on a specific hierarchy:
 * 1. Name (lexicographical order).
 * 2. Value (integer comparison) - if names are equal.
 * 3. Type (enum comparison) - if names and values are equal.
 * Returns < 0 if item1 < item2, > 0 if item1 > item2, and 0 if identical.
 */
int compareItems(void* a, void* b) {
    Item* item1 = (Item*)a;
    Item* item2 = (Item*)b;

    int res = strcmp(item1->name, item2->name);
    if (res == 0)
        res = item1->value - item2->value;

    if (res != 0)
        return res;

    return item1->type - item2->type;
}

/*
 * Compares two monsters based on hierarchy:
 * 1. Name (lexicographical).
 * 2. HP (integer comparison).
 * 3. Attack (integer comparison).
 * Returns < 0, > 0, or 0.
 */
int compareMonsters(void* a, void* b) {

    Monster* monster1 = (Monster*)a;
    Monster* monster2 = (Monster*)b;

    int res = strcmp(monster1->name, monster2->name);
    if (res == 0)
        res = monster1->hp - monster2->hp;

    if (res != 0)
        return res;

    return monster1->attack - monster2->attack;
}

/*
 * Frees the memory allocated for a specific item.
 * It first frees the dynamically allocated name string,
 * and then frees the Item struct itself.
 */
void freeItem(void* data) {

    Item* item = (Item*)data;
    //nothing to free
    if (item == NULL)
        return;

    //free feature
    if (item->name != NULL)
        free(item->name);

    free(item);
}

//free memory of monster pointer
void freeMonster(void* data) {
    Monster* mon = (Monster*)data;

    if (mon == NULL)
        return;

    if (mon->name != NULL) {
        free(mon->name);
    }

    free(mon);
}

/*
 * Prints the details of a specific monster (Name, HP, Attack).
 * Conforms to the generic function signature required by the BST.
 */
void printMonster(void* data) {

    if (data == NULL)
        return;

    Monster* mon = (Monster*)data;
    char* typeStr = getMonsterTypeString(mon->type);

    printf("[%s] Type: %s, Attack: %d, HP: %d\n",
        mon->name,
        typeStr,
        mon->attack,
        mon->hp);
}

void printItem(void* data) {
    //safety check
    if (data == NULL)
        return;

    Item* item = (Item*)data;
    char* itemType = getItemTypeString(item->type);

    printf("[%s] %s - Value: %d\n",
        itemType,
        item->name,
        item->value);
}

//returns string of the moster type
char* getMonsterTypeString(MonsterType monType) {
    switch (monType) {
    case PHANTOM:
        return "Phantom";
    case SPIDER:
        return "Spider";
    case DEMON:
        return "Demon";
    case GOLEM:
        return "Golem";
    case COBRA:
        return "Cobra";
    }
    return NULL;
}

//returns string of the Item Type
char* getItemTypeString(ItemType type) {
    switch (type) {
    case ARMOR:
        return "ARMOR";
    case SWORD:
        return "SWORD";
    default:
        return NULL;
    }
}
