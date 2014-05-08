/*
 * Game.c - data organisation system for the
 * Geographic Model of Disciplines Simulation
 * 
 * By Timothy Chin, James Houlahan, Matthew Siesco and Tianqi Liu
 * 
 * This file contains the handlers for the game simulation.
 * The interaction and frontend exists in runGame.c
 * This file only contains functions for use with manipulating
 * data stored in the game type. It provides the implementation of the
 * API given in Game.h.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "Game.h" // See Game.h for all API functions and structs


#define GRID_WIDTH 7
#define GRID_HEIGHT 6
#define NUM_ARCS_PER_HEX 3
#define NUM_VERTICES_PER_HEX 2
#define NUM_RETRAINS_PER_HEX 2
#define NUM_DISCIPLINES 6
#define OUTSIDE_BOARD -1


// =====================================================================
//    TYPEDEFS/STRUCTS BEGIN
// =====================================================================

// Represents a single coordinate within the 2d grid in our game struct.
// Simplifies the process of referring to a particular arc/vertex within
// the grid, since we can give the exact x/y coordinates plus which 
// arc/vertex in the hex we require
typedef struct _coord {
    int x;
    int y;

    // a coordinate should either represent an edge or a vertex, not
    // both, so set the unused one to -1. E.g. an arc should have -1
    // for its "vertNum"
    int arcNum;
    int vertNum;
} coord;


// Used throughout the implementation, stores a particular location in
// the 2d array. To access an element in the board specified as 
// a coordinate, use game.grid[coord.x][coord.y] and select the 
// appropriate arc or vertex by coord.arcNum or coord.vertNum
typedef struct _hex {
    // stores the resource type of the hex
    int resType;

    // stores the dice number that triggers this hex to produce students
    int diceNum;

    // stores what arcs (if any) are on the top three edges 
    int arcs[NUM_ARCS_PER_HEX];

    // stores what campuses/GO8s (if any) are on the top two corners
    int vertices[NUM_VERTICES_PER_HEX];

    // stores whether the top two corners are retraining points
    // and of what colour. Either 0 (empty) or one of the student types
    int retrainCenters[NUM_RETRAINS_PER_HEX];
} hex;


// This is the struct representing a given game state. It is passed
// throughout all the functions within the API. Avoid accessing
// the contents of this game struct directly -- if a function exists
// to retrieve what you are trying to retrieve, use it, don't access the
// struct unnecessarily
typedef struct _game {
    // a 7 wide by 6 high grid of the board, each element is type hex
    // grid[x][y]
    hex grid[GRID_WIDTH][GRID_HEIGHT];

    // what turn we are on
    int turnNumber;

    // the number of each resource type each uni has [A, B, C]
    // first index is uni number, second is student type
    int studentAmounts[NUM_UNIS][NUM_DISCIPLINES];

    // the number of each KPI points each uni has[A, B, C]
    int numKPI[NUM_UNIS];

    // the number of ARCs each uni has [A, B, C]
    // updated each time we build an arc
    int numARCs[NUM_UNIS];

    // the number of campuses each uni has [A, B, C]
    // updated each time we build a campus
    int numCampuses[NUM_UNIS];

    // the number of GO8s each uni has [A, B, C]
    // updated each time we upgrade a campus to a GO8
    int numGO8s[NUM_UNIS];

    // the number of IP patents each uni has [A, B, C]
    int numIPs[NUM_UNIS];

    // the number of publications each uni has [A, B, C]
    int numPubs[NUM_UNIS];

    // holds which uni has the most arcs and how many they have
    int uniWithMostARCs;
    int uniWithMostARCs_number;

    // holds which uni has the most publications and how many they have
    int uniWithMostPubs;
    int uniWithMostPubs_number;
} game;


// =====================================================================
//   TYPEDEFS/STRUCTS END
//   STATIC FUNCTION DECLARATIONS BEGIN
// =====================================================================

// Note: All functions that are here should be STATIC. We don't want
// anyone using our internal functions and messing with our stuff
// and we have to stick to the interface anyway.

// functions to convert between the regionID and our coordinate system
static coord regIDToCoord(int regID);
static int coordToRegID(coord inCoord);

// functions to return which arc (or vertex) is at the end of a path
static coord pathToARC(path inPath);
static coord pathToVertex(path inPath);

// function which steps through the path, beginning at the default
// starting point on the board, and if the path leaves the board at any 
// time, returns FALSE. If the path remains contained within the board
// it returns TRUE. "RRRRRR" is not contained, whereas "LRRRRR" is.
static int isPathContained(path inPath);

// when building a new campus, call "isCampusConnected" on the 
// destination vertex to ensure there are arcs adjacent. When building 
// an ARC, call isARCConnected on the destination edge to ensure an
// ARC or campus is adjacent
static int isARCConnected(path inPath,int player);
static int isCampusConnected(path inPath,int player);


// =====================================================================
//   STATIC FUNCTION DECLARATIONS END
//   STATIC FUNCTIONS BEGIN
// =====================================================================

// Return the coordinate of a hex given its region ID
static coord regIDToCoord(int regID) {
    coord newCoord;
    if (regID < 3) {
        newCoord.x = 1;
        newCoord.y = 5-regID;
    } else if (regID < 7) {
        newCoord.x = 2;
        newCoord.y = 5-regID+3;
    } else if (regID < 12) {
        newCoord.x = 3;
        newCoord.y = 5-regID+7;
    } else if (regID < 16) {
        newCoord.x = 4;
        newCoord.y = 5-regID+11;
    } else {
        newCoord.x = 5;
        newCoord.y = 5-regID+14;
    }

    return newCoord;
}


// return the region ID of a hex given it's coordinate, or -1 if 
// outside the board
static int coordToRegID(coord inCoord) {
    int newRegID;
    if (inCoord.x < 1) {
        newRegID = OUTSIDE_BOARD;
    } else if (inCoord.x < 2) {
        if (inCoord.y < 3) {
            newRegID = OUTSIDE_BOARD;
        } else {
            newRegID = 5 - inCoord.y;
        }
    } else if (inCoord.x < 3) {
        if (inCoord.y < 2) {
            newRegID = OUTSIDE_BOARD;
        } else {
            newRegID = 8 - inCoord.y;
        }
    } else if (inCoord.x < 4) {
        if (inCoord.y < 1) {
            newRegID = OUTSIDE_BOARD;
        } else {
            newRegID = 12 - inCoord.y;
        }
    } else if (inCoord.x < 5) {
        if (inCoord.y < 1 || inCoord.y > 4) {
            newRegID = OUTSIDE_BOARD;
        } else {
            newRegID = 16 - inCoord.y;
        }
    } else if (inCoord.x < 6) {
        if (inCoord.y < 1 || inCoord.y > 3) {
            newRegID = OUTSIDE_BOARD;
        } else {
            newRegID = 19 - inCoord.y;
        }
    } else {
        newRegID = OUTSIDE_BOARD;
    }

    return newRegID;
}

//returns the coordinate and the corresponding vertex of a given path
static coord pathToVertex(path inPath){
    int count = 0; // set a counter to determine which step
    coord initialCoord = {.x = 3,.y = 5,.arcNum = -1,.vertNum = 0};   
    int direct = 2; //each vertex(0,1) can have 3 directions, each direction has 3 path(L,B,R)
    
    while (inPath[count] != '\0'){
        switch (initialCoord.vertNum){
        case 0: //vertex 1 
            initialCoord.vertNum = 1;
            if (direct == 0){
                initialCoord.x -= 1;
                if (inPath[count] == 'L'){
                    direct = 2;
                }else if (inPath[count] == 'R'){
                    initialCoord.y += 1;
                    direct = 1;
                }else if (inPath[count] == 'B'){
                    initialCoord.x += 1;
                    direct = 0;
                }
            }else if (direct == 1){
                if (inPath[count]  == 'L'){
                    initialCoord.x -= 1;
                    initialCoord.y += 1;
                    direct = 1;
                }else if (inPath[count]  == 'R'){
                    direct = 0;
                }else if (inPath[count]  == 'B'){
                    initialCoord.x -= 1;
                    direct = 2;
                }
            }else if (direct == 2){
                initialCoord.vertNum = 1;
                if (inPath[count]  == 'L'){
                    direct = 0;
                }else if (inPath[count]  == 'R'){
                    initialCoord.x -= 1;
                    direct = 2;
                }else if (inPath[count]  == 'B'){
                    initialCoord.x -= 1;
                    initialCoord.y += 1;
                    direct = 1;
                }
            }
            count++;break;
        case 1: //vertex 2
            initialCoord.vertNum = 0;
            if (direct == 0){
                initialCoord.x += 1;
                if (inPath[count] == 'L'){
                    direct = 1;
                }else if (inPath[count]  == 'R'){
                    initialCoord.y -= 1;
                    direct = 2;
                }else if (inPath[count]  == 'B'){
                    initialCoord.x -= 1;
                    direct = 0;
                }
            }else if (direct == 1){
                if (inPath[count]  == 'L'){
                    direct = 0;
                }else if (inPath[count]  == 'R'){
                    initialCoord.x += 1;
                    direct = 1;
                }else if (inPath[count]  == 'B'){
                    initialCoord.x += 1;
                    initialCoord.y -= 1;
                    direct = 2;
                }
            }else if (direct == 2){
                if (inPath[count] == 'L'){
                    initialCoord.x += 1;
                    initialCoord.y -= 1;
                    direct = 2;
                }else if (inPath[count] == 'R'){
                    direct = 0;
                }else if (inPath[count]  == 'B'){
                    initialCoord.x += 1;
                    direct = 1;
                }
            }
            count++;break;
        default:break;
        }
    }
    return initialCoord;
}

static coord pathToARC(path inPath){
    int count = 0;
    coord initialCoord = { .x = 3, .y = 5, .arcNum = -1, .vertNum = 0 };
    //vertNum set to 0 or 1 to determine which vertex currently on, doesn't matter what 
    //coordinate
    //arcNum set to -1 because initially there is no arc, so it returns -1 if player 
    //didn't enter a path

    int direct = 2; //three directions 0,1,2 

    while (inPath[count]!='\0')
    {
        switch (initialCoord.vertNum){
        case 0:
            initialCoord.vertNum = 1;
            if (direct == 0){
                if (inPath[count] == 'L'){
                    initialCoord.arcNum = 0;
                    direct = 2;
                  
                }else if (inPath[count] == 'R'){
                    initialCoord.x -= 1;
                    initialCoord.y += 1;
                    initialCoord.arcNum = 2;
                    direct = 1;
                }else if (inPath[count] == 'B'){
                    direct = 0;
                    initialCoord.arcNum = 1;
                }
            }else if (direct == 1){
                if (inPath[count] == 'L'){
                    initialCoord.x -= 1;
                    initialCoord.y += 1;
                    initialCoord.arcNum = 2;
                    direct = 1;
                }else if (inPath[count] == 'R'){
                    direct = 0;
                    initialCoord.arcNum = 1;
                }else if (inPath[count] == 'B'){
                    direct = 2;
                    initialCoord.arcNum = 0;
                }
            }else if (direct == 2){
                initialCoord.vertNum = 1;
                if (!(initialCoord.x == 3 && initialCoord.y == 5)){
                    initialCoord.x += 1;
                    initialCoord.y -= 1;
                }if (inPath[count] == 'L'){
                    direct = 0;
                    initialCoord.arcNum = 1;
                }else if (inPath[count] == 'R'){
                    direct = 2;
                    initialCoord.arcNum = 0;
                }else if (inPath[count] == 'B'){
                    initialCoord.x -= 1;
                    initialCoord.y += 1;
                    direct = 1;
                    initialCoord.arcNum = 2;
                }
            }
            count++;break;
        case 1:
            initialCoord.vertNum = 0;
            if (direct == 0){
                if (inPath[count] == 'L'){
                    initialCoord.x += 1;
                    direct = 1;
                    initialCoord.arcNum = 0;
                }else if (inPath[count] == 'R'){
                    initialCoord.arcNum = 2;
                    direct = 2;
                }else if (inPath[count] == 'B'){
                    initialCoord.arcNum = 1;
                    direct = 0;
                }
            }else if (direct == 1){
                if (inPath[count] == 'L'){
                    direct = 0;
                    initialCoord.arcNum = 1;
                }else if (inPath[count] == 'R'){
                    initialCoord.x += 1;
                    direct = 1;
                    initialCoord.arcNum = 0;
                }else if (inPath[count] == 'B'){
                    direct = 2;
                    initialCoord.arcNum = 2;
                }
            }else if (direct == 2){
                initialCoord.x -= 1;
                if (inPath[count] == 'L'){
                    direct = 2;
                    initialCoord.arcNum = 2;
                }else if (inPath[count] == 'R'){
                    direct = 0;
                    initialCoord.arcNum = 1;
                }else if (inPath[count] == 'B'){
                    initialCoord.x += 1;
                    direct = 1;
                    initialCoord.arcNum = 0;
                }
            }
            count++;break;
        default:break;
        }
    }
    initialCoord.vertNum = -1;
    return initialCoord;
}

// Move an imaginary point along the path. If at any time the point 
// exits the game board, it means the path ISN'T contained. If the 
// point remains within the board for the duration of the path, then
// the path IS contained.
static int isPathContained(path inPath) {
    // there are six directions we could be facing, call them 0..5
    // turning RIGHT adds 1 mod 6
    // turning LEFT subtracts 1 mod 6
    // turning BACK adds 3 mod 6
    // DOWN-RIGHT: 0    DOWN-LEFT: 1
    // LEFT: 2          UP-LEFT: 3
    // UP-RIGHT: 4      RIGHT: 5

    // suppose we are outside the board, about to step onto the board
    coord currentCoord = {.x = 2, .y = 6};
    int currentDirection = 0;
    int isContained = TRUE;

    // traverse the whole path as long as we remain inside the board
    int i = 0;
    while (inPath[i] != 0 && isContained == TRUE) {
        // read the direction to turn, assert it is a valid direction
        char turn = inPath[i];
        assert(turn == 'R' || turn == 'L' || turn == 'B');

        // newDirection is the direction [0..5] we face after the turn
        int newDirection;
        if (turn == 'L') {
            newDirection = (currentDirection - 1 + 6) % 6;
        } else if (turn == 'R') {
            newDirection = (currentDirection + 1) % 6;
        } else {
            newDirection = (currentDirection + 3) % 6;
        }

        // depending on the turn, adjust the current coordinate
        if (newDirection == 1 && currentDirection == 0) {
            currentCoord.x++;
            currentCoord.y--;
        } else if (newDirection == 2 && currentDirection == 1) {
            currentCoord.x--;
        } else if (newDirection == 3 && currentDirection == 2) {
            currentCoord.x--;
            currentCoord.y++;
        } else if (newDirection == 4 && currentDirection == 3) {
            currentCoord.x++;
        } else if (newDirection == 5 && currentDirection == 0) {
            currentCoord.x++;
            currentCoord.y--;
        } else if (newDirection == 4 && currentDirection == 5) {
            currentCoord.x++;
        } else if (newDirection == 3 && currentDirection == 4) {
            currentCoord.x--;
            currentCoord.y++;
        } else if (newDirection == 0 && currentDirection == 1) {
            currentCoord.x--;
        } 

        // coordToRegID returns -1 if outside board so we can use that 
        // to our advantage to check if the coordinate we ended up at
        // is inside the board or not
        if (coordToRegID(currentCoord) == -1) {
            isContained = FALSE;
        }

        // update the direction we face
        currentDirection = newDirection;
        i++;
    }

    return isContained;
}


// =====================================================================
//   STATIC FUNCTIONS END
//   API FUNCTIONS BEGIN
// =====================================================================

// create a new game struct on the heap, set up its initial values such
// as the hex types as given by the discipline[] and dice[] arrays, and
// return a Game variable holding a pointer to it
Game newGame (int discipline[], int dice[]) {
    Game g;
    return g;
}


// free all the memory malloced for the game
void disposeGame (Game g) {
    free(g);
}


// make the specified action for the current player and update the 
// game state accordingly.  
// The function may assume that the action requested is legal.
// START_SPINOFF is not a legal action here
void makeAction (Game g, action a) {
    // perform the requested action `a'
    // update counters as required (e.g. uniWithMostPubs, numIPs, 
    // numKPI, studentAmounts etc)
}


// advance the game to the next turn, 
// assuming that the dice has just been rolled and produced diceScore
// the game starts in turn -1 (we call this state "Terra Nullis") and 
// moves to turn 0 as soon as the first dice is thrown. 
void throwDice (Game g, int diceScore) {
    // advance the turn and distribute resources
}


// what type of students are produced by the specified region?
// regionID is the index of the region in the newGame arrays (above) 
// see discipline codes above
int getDiscipline (Game g, int regionID) {
    coord c = regIDToCoord(regionID);
    return g->grid[c.x][c.y].resType;
}


// what dice value produces students in the specified region?
// 2..12
int getDiceValue (Game g, int regionID) {
    coord c = regIDToCoord(regionID);
    return g->grid[c.x][c.y].diceNum;
}


// which university has the prestige award for the most ARCs?
// this is NO_ONE until the first arc is purchased after the game 
// has started.  
int getMostARCs (Game g) {
    return g->uniWithMostARCs;
}


// which university has the prestige award for the most pubs?
// this is NO_ONE until the first publication is made.
int getMostPublications (Game g) {
    return g->uniWithMostPubs;
}


// return the current turn number of the game -1,0,1, ..
int getTurnNumber (Game g) {
    return g->turnNumber;
}


// return the player id of the player whose turn it is 
// the result of this function is NO_ONE during Terra Nullis
int getWhoseTurn (Game g) {
    int p;
    if (getTurnNumber(g) == -1) {
        p = NO_ONE;
    } else {
        p = getTurnNumber(g) % 3 + 1;
    }
    return p;
}


// return the contents of the given vertex (ie campus code or 
// VACANT_VERTEX)
int getCampus(Game g, path pathToVertex) {
    // return what exists on the given vertex
    int vertex;
    return vertex;
}


// return the contents of the given edge (ie ARC code or vacant ARC)
int getARC(Game g, path pathToEdge) {
    // return what exists on the given ARC
    int arc;
    return arc;
}


// returns TRUE if it is legal for the current player to make the 
// specified action, FALSE otherwise.
// It is not legal for a player to make the moves OBTAIN_PUBLICATION 
// or OBTAIN_IP_PATENT (they can make the move START_SPINOFF)
// you can assume that any paths passed in are NULL terminated strings.
int isLegalAction (Game g, action a) {
    // assume the action is legal, then go through a series of tests,
    // changing it to FALSE if it fails a test.
    int isLegal = TRUE;

    // example: players cannot outright ask to obtain ip/publication
    if (a.actionCode == OBTAIN_PUBLICATION 
            || a.actionCode == OBTAIN_IP_PATENT) {
        isLegal = FALSE;
    }
    return isLegal;
}


// return the number of KPI points the specified player has
int getKPIpoints (Game g, int player) {
    // player-1 so that we can use the player number 1..3 as index 0..2
    return g->numKPI[player-1];
}


// return the number of ARC grants the specified player has
int getARCs (Game g, int player) {
    // player-1 so that we can use the player number 1..3 as index 0..2
    return g->numARCs[player-1];
}


// return the number of GO8 campuses the specified player has
int getGO8s (Game g, int player) {
    // player-1 so that we can use the player number 1..3 as index 0..2
    return g->numGO8s[player-1];
}


// return the number of normal Campuses the specified player has
int getCampuses (Game g, int player) {
    // player-1 so that we can use the player number 1..3 as index 0..2
    return g->numCampuses[player-1];
}


// return the number of IP Patents the specified player has
int getIPs (Game g, int player) {
    // player-1 so that we can use the player number 1..3 as index 0..2
    return g->numIPs[player-1];
}


// return the number of Publications the specified player has
int getPublications (Game g, int player) {
    // player-1 so that we can use the player number 1..3 as index 0..2
    return g->numPubs[player-1];
}


// return the number of students of the specified discipline type 
// the specified player has
int getStudents (Game g, int player, int discipline) {
    // player-1 so that we can use the player number 1..3 as index 0..2
    // use the discipline 0..5 as the index
    return g->studentAmounts[player-1][discipline];
}


// return how many students of discipline type disciplineFrom
// the specified player would need to retrain in order to get one 
// student of discipline type disciplineTo.  This will depend 
// on what retraining centers, if any, they have a campus at.
int getExchangeRate (Game g, int player, 
                     int disciplineFrom, int disciplineTo) {
    // assume exchange rate is 3, search through entire g-grid[x][y] 
    // and if you find a retraining center of the correct type with a 
    // campus owned by player at it, change it to 2
    int e = 3;
    // *** do search ***
    return e;
}

