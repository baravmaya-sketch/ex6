#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "utils.h"

#define LEGEND_MONSTER 'M'
#define LEGEND_ITEM    'I'
#define LEGEND_PRESENT 'V'
#define LEGEND_ABSENT  'X'

//static functions
static void displayMap(GameState* g);
static void computeNewCoords(int roomDirec, int* x, int* y);
static Room* findRoomById(Room* head, int roomID);
static int isRoomOccupied(Room* head, int x, int y);
static void printOrderOptions(GameState* g, BST* tree, void (*printFunc)(void*));
static int checkWinCondition(GameState* g);
static void handleWin(GameState* g);

typedef enum { MOVE = 1, FIGHT = 2, PICKUP = 3, 
               BAG = 4, DEFEATED = 5, QUIT = 6 } GameAction;

typedef enum { PREORDER = 1, INORDER = 2, POSTORDER = 3 } Order;

// Print the game legend (which rooms contain monsters/items)
static void printLegend(Room* room) {//change to room*
    printf("=== ROOM LEGEND ===\n");

    // Iterate over the linked list of rooms
    for (Room* r = room; r != NULL; r = r->next) {

        // Determine status chars based on existence (V or X)
        char mStatus = (r->monster != NULL) ? LEGEND_PRESENT : LEGEND_ABSENT;
        char iStatus = (r->item != NULL) ? LEGEND_PRESENT : LEGEND_ABSENT;

        // Print using the defined constants for M/I and V/X
        // Format: ID 1: [M:X] [I:V]
        printf("ID %d: [%c:%c] [%c:%c]\n",
            r->id,
            LEGEND_MONSTER, mStatus,
            LEGEND_ITEM, iStatus);
    }

    printf("===================\n");
}

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

// Displays the current game map and room legend
void displayGameStatus(GameState* g) {
    displayMap(g);
    printLegend(g->rooms);
}

// Displays current room details and player status
void displayRoomAndPlayerStatus(GameState* g) {
    //safety check
    if (g == NULL || g->player == NULL || g->player->currentRoom == NULL)
        return;

    Room* currRoom = g->player->currentRoom;

    printf("--- Room %d ---\n", currRoom->id);
    
    if (currRoom->monster)
        printf("Monster: %s (HP:%d)\n", currRoom->monster->name, currRoom->monster->hp);
    
    if (currRoom->item)
        printf("Item: %s\n", currRoom->item->name);
    
    printf("HP: %d/%d\n", g->player->hp, g->player->maxHp);
}

// Creates a new room adjacent to an existing room and optionally adds a monster or item
void addRoom(GameState* g) {
    displayGameStatus(g);
    int baseId = getInt("Attach to room ID", g);
    Room* baseRoom = findRoomById(g->rooms, baseId);


    int roomDirec = getInt(stringChooseDirection(), g);


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
    newRoom->next = NULL;
    
    if (g->rooms == NULL) {
        g->rooms = newRoom;
    }
    else {
        Room* temp = g->rooms;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newRoom;
    }

    int addMonster = getInt("Add monster? (1=Yes, 0=No):", g);
    if (addMonster) {
        addMonsterFunc(newRoom, g);
    }

    int addItem = getInt("Add item? (1=Yes, 0=No):", g);
    if (addItem)
        addItemFunc(newRoom, g);

    printf("Created room %d at (%d, %d)", newRoom->id, newRoom->x, newRoom->y);
}

// Helper function to allocate and initialize a monster in the room
void addMonsterFunc(Room* room, GameState* g) {
    // Allocate memory for the monster struct
    room->monster = (Monster*)malloc(sizeof(Monster));
    if (room->monster == NULL) {
        exit(1);
    }

    // Get monster details from user
    room->monster->name = getString("Monster name: ");
    room->monster->type = (MonsterType)getInt("Type (0-4): ", g);
    // Using getInt for safe integer input
    room->monster->hp = getInt("HP: ",g);
    room->monster->attack = getInt("Attack:",g);
}

// Helper function to allocate and initialize an item in the room
void addItemFunc(Room* room, GameState* g) {
    // Allocate memory for the item struct
    room->item = (Item*)malloc(sizeof(Item));
    if (room->item == NULL) {
        exit(1);
    }

    // Get item details from user
    room->item->name = getString("Item name: ");
    // Assuming ItemType is an enum (0-3), we cast the integer input
    room->item->type = (ItemType)getInt("Type (0=Armor, 1=Sword):", g);
    room->item->value = getInt("Value: ", g);
}

// Updates coordinates based on the chosen movement direction
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
    g->player->currentRoom->visited = 1;
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

// Prints item details in a formatted manner
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

// Main game loop handling player actions and game flow
void playGame(GameState* g) {
    GameAction choice = 0;
    while (choice != QUIT) {
        displayGameStatus(g);
        displayRoomAndPlayerStatus(g);
        printGameOptions();
        
        choice = (GameAction)getInt(NULL, g);
        Player* player = g->player;
        Room* currRoom = player->currentRoom;
        Monster* monster = currRoom->monster;
        
        switch (choice)
        {
            case MOVE:
            {
                currRoom->visited = 1;
                
                if (monster) {
                    printf("Kill monster first\n");
                    break;
                }
                
                int roomDirec = getInt(stringChooseDirection(), g);

                int targetX = currRoom->x;
                int targetY = currRoom->y;
                computeNewCoords(roomDirec, &targetX, &targetY);
                Room* targetRoom = findRoomByCoords(g->rooms, targetX, targetY);
                
                if (!targetRoom) {
                    printf("No room there\n");
                    break;
                }
                
                player->currentRoom = targetRoom;
                if (checkWinCondition(g))
                    handleWin(g);
                
                break;
            }

            case FIGHT:
            {
                int playerWon = 0;
                if (monster == NULL) {
                    printf("No monster\n");
                    break;
                }
                while (player->hp > 0) {
                    monster->hp = monster->hp - player->baseAttack;
                    printf("You deal %d damage. Monster HP: %d\n", 
                        player->baseAttack, monster->hp > 0 ? monster->hp : 0);
                    if (monster->hp <= 0) {
                        playerWon = 1;
                        break;
                    }
                    else {
                        player->hp = player->hp - monster->attack;
                        printf("Monster deals %d damage. Your HP: %d\n", 
                            monster->attack, player->hp > 0 ? player->hp : 0);
                    }

                }
                if (playerWon == 0) {
                    freeGame(g);
                    printf("--- YOU DIED ---");
                    exit(0);
                }
                printf("Monster defeated!\n");
                bstInsert(player->defeatedMonsters, monster, compareMonsters);
                currRoom->monster = NULL;
                if (checkWinCondition(g)) {
                    handleWin(g);
                }
                break;
            }

            case PICKUP:
            {
                if (monster) {
                    printf("Kill monster first\n");
                    break;
                }

                if (currRoom->item == NULL) {
                    printf("No item here\n");
                    break;
                }
                void* found = bstFind(player->bag, currRoom->item, compareItems);
                if (found) {
                    printf("Duplicate item.\n");
                    break;
                }
                bstInsert(player->bag, currRoom->item, compareItems);
                printf("picked up %s", currRoom->item->name);
                currRoom->item = NULL;

                break;
            }

            case BAG:
            {
                printf("=== INVENTORY ===\n");
                printOrderOptions(g, player->bag, printItem);
                break;
            }

            case DEFEATED:
            {
                printf("=== DEFEATED MONSTERS ===\n");
                printOrderOptions(g, player->defeatedMonsters, printMonster);
                break;
            }
        }     
    }
}

// Prints the main game action menu
void printGameOptions() {
    printf("1.Move 2.Fight 3.Pickup 4.Bag 5.Defeated 6.Quit\n");
}

// Finds and returns a room by its X and Y coordinates
Room* findRoomByCoords(Room* head, int x, int y) {
    for (Room* r = head; r != NULL; r = r->next) {
        if (r->x == x && r->y == y) {
            return r;
        }
    }
    return NULL;
}

/*frees the memory of player except of room
(that free game state will do)*/
static void freePlayer(Player* player) {

    //safety check
    if (player == NULL)
        return;

    bstFree(player->bag, freeItem);
    bstFree(player->defeatedMonsters, freeMonster);
    free(player);
}

// Frees a room and its associated monster and item
static void freeRoom(Room* room) {
    //safety check
    if (room == NULL)
        return;

    if (room->monster)
        freeMonster(room->monster);

    if (room->item)
        freeItem(room->item);

    free(room);
}

// Frees all game resources including player and rooms
static void freeGameState(GameState* game) {
    if (!game)
        return;

    //free player
    if (game->player)
        freePlayer(game->player);

    Room* iterRoom = game->rooms;
    while (iterRoom != NULL) {
        Room* nextRoom = iterRoom->next;
        freeRoom(iterRoom);
        iterRoom = nextRoom;
    }

    free(game);
}

// Wrapper function to free the entire game state
void freeGame(GameState* g) {
    freeGameState(g);
}

// Returns a prompt string for choosing movement direction
char* stringChooseDirection()
{
    return "Direction (0=Up,1=Down,2=Left,3=Right):\n";
}

// Safely reads an integer input and exits cleanly on failure
int getInt(char* prompt, GameState* gameState) {
    int outNum = 0;
    if (getIntInternal(prompt, &outNum) == 0)
    {
        freeGame(gameState);
        exit(0);
    }

    return outNum;

}

// Prompts for BST traversal order and prints the tree accordingly
static void printOrderOptions(GameState* g, BST* tree, void (*printFunc)(void*)) {

    Order orderChoice = (Order)getInt("1.Preorder 2.Inorder 3.Postorder\n", g);

    switch (orderChoice) {
    case PREORDER:
        bstPreorder(tree, printFunc);
        break;

    case INORDER:
        bstInorder(tree, printFunc);
        break;

    case POSTORDER:
        bstPostorder(tree, printFunc);
        break;
    }
}

// Checks if all rooms were visited and all monsters defeated
static int checkWinCondition(GameState* g) {
    Room* iterRoom = g->rooms;
    while (iterRoom != NULL) {
        if (iterRoom->monster || iterRoom->visited == 0)
            return 0;

        iterRoom = iterRoom->next;
    }
    return 1;
}

// Handles game completion and victory state
static void handleWin(GameState* g) {
    printf("********************************************\n");
    printf("                  VICTORY!                  \n");
    printf(" All rooms explored. All monsters defeated. \n");
    printf("********************************************\n");
    freeGame(g);
    exit(0);
}
