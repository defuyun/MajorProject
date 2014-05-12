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
#define NUM_RETRAINING_CENTRES 10
#define DISCOUNT_EXCHANGE_RATE 2
#define DEFAULT_EXCHANGE_RATE 3
#define OUTSIDE_BOARD -1

#define CAMPUS_KPI 10
#define GO8_KPI 20
#define ARC_KPI 2
#define IP_KPI 10
#define PRESTIGE_BONUS 10



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
static int isARCConnected(path inPath, Game g, int player);
static int isCampusConnected(path inPath, Game g, int player);

// returns true if there are campuses next to the vertex at inPath
static int isCampusTooClose(Game g, path inPath);

// returns true if the coordinate is inside the board
static int isCoordInside(coord c);


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
    if (isCoordInside(inCoord) == FALSE) {
        newRegID = -1;
    } else if (inCoord.x < 2) {
        newRegID = 5 - inCoord.y;
    } else if (inCoord.x < 3) {
        newRegID = 8 - inCoord.y;
    } else if (inCoord.x < 4) {
        newRegID = 12 - inCoord.y;
    } else if (inCoord.x < 5) {
        newRegID = 16 - inCoord.y;
    } else {
        newRegID = 19 - inCoord.y;
    }

    return newRegID;
}

// return the coordinate of a path at the end of the vertex
static coord pathToVertex(path inPath) {
    // there are six directions we could be facing, call them 0..5
    // turning RIGHT adds 1 to the direction, mod 6
    // turning LEFT subtracts 1 off the direction, mod 6

    // start at the initial position above the grid
    coord finalCoord = {.x = 2, .y = 6};

    int i = 0;
    // preDirection is the direction [0..5] we face before the turn
    int preDirection = 0;
    while (inPath[i] != 0) {
        char turn = inPath[i];
        // postDirection is the direction [0..5] we face after the turn
        int postDirection;
        if (turn == 'L') {
            postDirection = (preDirection - 1) % 6;
        } else if (turn == 'R') {
            postDirection = (preDirection + 1) % 6;
        } else {
            postDirection = (preDirection + 3) % 6;
        }
        postDirection %= 6;
        if (postDirection < 0) {
            postDirection += 6;
        }

        if (postDirection == 1 && preDirection == 0) {
            finalCoord.x++;
            finalCoord.y--;
        } else if (postDirection == 2 && preDirection == 1) {
            finalCoord.x--;
        } else if (postDirection == 3 && preDirection == 2) {
            finalCoord.x--;
            finalCoord.y++;
        } else if (postDirection == 4 && preDirection == 3) {
            finalCoord.x++;
        } else if (postDirection == 5 && preDirection == 0) {
            finalCoord.x++;
            finalCoord.y--;
        } else if (postDirection == 4 && preDirection == 5) {
            finalCoord.x++;
        } else if (postDirection == 3 && preDirection == 4) {
            finalCoord.x--;
            finalCoord.y++;
        } else if (postDirection == 0 && preDirection == 1) {
            finalCoord.x--;
        } else {
            // Do nothing
        }

        // update the direction we face
        preDirection = postDirection;
        i++;
    }

    // alter the coordinates so we can store the vertex on the end
    if (preDirection == 0) {
        finalCoord.x++;
        finalCoord.y--;
        finalCoord.vertNum = 0;
    } else if (preDirection == 1) {
        finalCoord.x--;
        finalCoord.vertNum = 1;
    } else if (preDirection == 2) {
        finalCoord.vertNum = 0;
    } else if (preDirection == 3) {
        finalCoord.vertNum = 1;
    } else if (preDirection == 4) {
        finalCoord.vertNum = 0;
    } else {
        finalCoord.vertNum = 1;
    }
    finalCoord.arcNum = -1;

    return finalCoord;
}

// return the coordinate of the arc at the end of a path
static coord pathToARC(path inPath) {
    // there are six directions we could be facing, call them 0..5
    // turning RIGHT adds 1 to the direction, mod 6
    // turning LEFT subtracts 1 off the direction, mod 6

    // start at the initial position above the grid
    coord finalCoord = {.x = 2, .y = 6};

    int i = 0;
    // preDirection is the direction [0..5] we face before the turn
    int preDirection = 0;
    while (inPath[i] != 0) {
        char turn = inPath[i];
        // postDirection is the direction [0..5] we face after the turn
        int postDirection;
        if (turn == 'L') {
            postDirection = (preDirection - 1) % 6;
        } else if (turn == 'R') {
            postDirection = (preDirection + 1) % 6;
        } else {
            postDirection = (preDirection + 3) % 6;
        }
        postDirection %= 6;
        if (postDirection < 0) {
            postDirection += 6;
        }

        if (postDirection == 1 && preDirection == 0) {
            finalCoord.x++;
            finalCoord.y--;
        } else if (postDirection == 2 && preDirection == 1) {
            finalCoord.x--;
        } else if (postDirection == 3 && preDirection == 2) {
            finalCoord.x--;
            finalCoord.y++;
        } else if (postDirection == 4 && preDirection == 3) {
            finalCoord.x++;
        } else if (postDirection == 5 && preDirection == 0) {
            finalCoord.x++;
            finalCoord.y--;
        } else if (postDirection == 4 && preDirection == 5) {
            finalCoord.x++;
        } else if (postDirection == 3 && preDirection == 4) {
            finalCoord.x--;
            finalCoord.y++;
        } else if (postDirection == 0 && preDirection == 1) {
            finalCoord.x--;
        } 

        // update the direction we face
        preDirection = postDirection;
        i++;
    }

    // return which arc is needed to hold our position in the hex
    finalCoord.arcNum = (preDirection % 3) - 1;
    if (finalCoord.arcNum < 0) {
        finalCoord.arcNum += 3;
    }
    finalCoord.vertNum = -1;

    return finalCoord;
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
    coord currentCoord = {.x = 2, .y = 6, .vertNum = -1};
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
            currentCoord.arcNum = 0;
        } else if (newDirection == 2 && currentDirection == 1) {
            currentCoord.x--;
            currentCoord.arcNum = 1;
        } else if (newDirection == 3 && currentDirection == 2) {
            currentCoord.x--;
            currentCoord.y++;
            currentCoord.arcNum = 2;
        } else if (newDirection == 4 && currentDirection == 3) {
            currentCoord.x++;
            currentCoord.arcNum = 0;
        } else if (newDirection == 5 && currentDirection == 0) {
            currentCoord.x++;
            currentCoord.y--;
            currentCoord.arcNum = 1;
        } else if (newDirection == 4 && currentDirection == 5) {
            currentCoord.x++;
            currentCoord.arcNum = 0;
        } else if (newDirection == 3 && currentDirection == 4) {
            currentCoord.x--;
            currentCoord.y++;
            currentCoord.arcNum = 2;
        } else if (newDirection == 0 && currentDirection == 1) {
            currentCoord.x--;
            currentCoord.arcNum = 2;
        } 

        // check to see if our coordinate ends up outside the board
        if (isCoordInside(currentCoord) == FALSE) {
            isContained = FALSE;
        }

        // update the direction we face
        currentDirection = newDirection;
        i++;
    }

    return isContained;
}


// Returns true if there are campuses adjacent to the vertex located at
// path "inPath"
static int isCampusTooClose(Game g, path inPath) {
    int isTooClose = FALSE;
    coord c = pathToVertex(inPath);
    if (c.vertNum == 0) {
        if (g->grid[c.x][c.y].vertices[1] != VACANT_VERTEX) {
            isTooClose = TRUE;
        } 
        if (g->grid[c.x-1][c.y].vertices[1] != VACANT_VERTEX) {
            isTooClose = TRUE;
        } 
        if (g->grid[c.x-1][c.y+1].vertices[1] != VACANT_VERTEX) {
            isTooClose = TRUE;
        }
    } else if (c.vertNum == 1) {
        if (g->grid[c.x][c.y].vertices[0] != VACANT_VERTEX) {
            isTooClose = TRUE;
        } 
        if (g->grid[c.x+1][c.y].vertices[0] != VACANT_VERTEX) {
            isTooClose = TRUE;
        }
        if (g->grid[c.x+1][c.y-1].vertices[0] != VACANT_VERTEX) {
            isTooClose = TRUE;
        }
    }

    return isTooClose;
}


// Returns true if the coordinate is inside the board, false otherwise
// the original way of doing this was to check if coordToRegID returned
// -1, however that wouldn't work for hexes that lay outside the board
// but had their top edges inside the board
static int isCoordInside(coord c) {
    int isInside = FALSE;
    
    // if we are only looking at the hex itself, not its arcs/vertices
    if (c.vertNum == c.arcNum && c.vertNum == -1) {
        if (c.x == 1 && c.y > 2 && c.y < 6) {
            isInside = TRUE;
        } else if (c.x == 2 && c.y > 1 && c.y < 6) {
            isInside = TRUE;
        } else if (c.x == 3 && c.y > 0 && c.y < 6) {
            isInside = TRUE;
        } else if (c.x == 4 && c.y > 0 && c.y < 5) {
            isInside = TRUE;
        } else if (c.x == 5 && c.y > 0 && c.y < 4) {
            isInside = TRUE;
        }
    } else {
        // if we are clearly inside the board
        if (c.x == 1 && c.y > 2 && c.y < 6) {
            isInside = TRUE;
        } else if (c.x == 2 && c.y > 1 && c.y < 6) {
            isInside = TRUE;
        } else if (c.x == 3 && c.y > 0 && c.y < 6) {
            isInside = TRUE;
        } else if (c.x == 4 && c.y > 0 && c.y < 5) {
            isInside = TRUE;
        } else if (c.x == 5 && c.y > 0 && c.y < 4) {
            isInside = TRUE;
        }

        // if we are on the left edge of the board, hex is outside
        if (c.x == 0 && c.y > 2 && c.y < 6 
                && (c.arcNum == 2 || c.vertNum == 1)) {
            isInside = TRUE;
        }

        // if we are on the right edge of the board, hex is outside
        if (c.x == 6 && c.y > -1 && c.y < 3 
                && (c.arcNum == 0 || c.vertNum == 0)) {
            isInside = TRUE;
        }

        // if we are on the right edge of the board, hex is outside
        if (c.x == 6 && c.y > -1 && c.y < 3 
                && (c.arcNum == 0 || c.vertNum == 0)) {
            isInside = TRUE;
        }

        // if we are on the bottom right edge of the board, hex is outside
        if (c.y == 0 && c.x > 2 && c.y < 6) {
            if (c.x == 3) {
                if (c.arcNum == 1 || c.vertNum == 0 || c.vertNum == 1) {
                    isInside = TRUE;
                }
            } else {
                if (c.arcNum == 0 || c.arcNum == 1 || c.vertNum == 0 
                        || c.vertNum == 1) {
                    isInside = TRUE;
                }
            }
        }

        // if we are on the bottom left edge of the board, hex is outside
        if (c.x + c.y == 3 && c.x < 3 && c.x > 0
                && (c.arcNum == 1 || c.arcNum == 2 || c.vertNum == 0 
                    || c.vertNum == 1)) {
            isInside = TRUE;
        }
    }

    return isInside;
}


static int isARCConnected(path inPath, Game g, int player){
   int connected = FALSE;
   path pL,pR,pBL,pBR,v0,v1;
   strcpy(pL, inPath);
   strcat(pL, "L");
   strcpy(pR, inPath);
   strcat(pR, "R");
   strcpy(pBL, inPath);
   strcat(pBL, "BL");
   strcpy(pBR, inPath);
   strcat(pBR, "BR");
   strcpy(v1, inPath);
   strcat(v1, "B");
   strcpy(v0, inPath);

   if (getARC(g,pL) == player || getARC(g,pR) == player
       || getARC(g,pBL) == player || getARC(g,pBR) == player
       || getCampus(g,v0) == player || getCampus(g,v1) == player) {
      connected = TRUE;
   }
   return connected;
}


static int isCampusConnected(path inPath, Game g, int player){
   int connected = FALSE;
   path pL;
   path pR;
   strcpy(pL, inPath);
   strcat(pL, "L");
   strcpy(pR, inPath);
   strcat(pR, "R");

   if (getARC(g,pL) == player || getARC(g,pR) == player
       || getARC(g,inPath) == player) {
      connected = TRUE;
   }
   return connected;
}








// =====================================================================
//   STATIC FUNCTIONS END
//   API FUNCTIONS BEGIN
// =====================================================================

// create a new game struct on the heap, set up its initial values such
// as the hex types as given by the discipline[] and dice[] arrays, and
// return a Game variable holding a pointer to it
Game newGame (int discipline[], int dice[]) {
    Game g = malloc(sizeof(game));

    // turn number starts at -1
    g->turnNumber = -1;

    int student = 0;
    while (student < NUM_UNIS) {
        // create the initial student numbers
        g->studentAmounts[student][STUDENT_THD] = 0;
        g->studentAmounts[student][STUDENT_BPS] = 3;
        g->studentAmounts[student][STUDENT_BQN] = 3;
        g->studentAmounts[student][STUDENT_MTV] = 1;
        g->studentAmounts[student][STUDENT_MJ] = 1;
        g->studentAmounts[student][STUDENT_MMONEY] = 1;
        
        // create the initial stats
        g->numKPI[student] = 20;
        g->numARCs[student] = 0;
        g->numCampuses[student] = 2;
        g->numGO8s[student] = 0;
        g->numIPs[student] = 0;
        g->numPubs[student] = 0;
        student++;
    }


    // each hex needs to be given the correct discipline and dice value
    int row = 0;
    int column = 0;
    // create the hex layout's disciplines and dice
    while (row < GRID_HEIGHT) {
        while (column < GRID_WIDTH) {
            coord currentCoord = {.x = column, .y = row, .arcNum = -1, 
                .vertNum = -1};
            if (isCoordInside(currentCoord)) {
                g->grid[column][row].resType 
                    = discipline[coordToRegID(currentCoord)];
                g->grid[column][row].diceNum 
                    = dice[coordToRegID(currentCoord)];
                g->grid[column][row].arcs[0] = VACANT_ARC;
                g->grid[column][row].arcs[1] = VACANT_ARC;
                g->grid[column][row].arcs[2] = VACANT_ARC;
                g->grid[column][row].vertices[0] = VACANT_VERTEX;
                g->grid[column][row].vertices[1] = VACANT_VERTEX;
            } else {
                g->grid[column][row].resType = -1;
                g->grid[column][row].diceNum = -1;
                g->grid[column][row].vertices[0] = VACANT_VERTEX;
                g->grid[column][row].vertices[1] = VACANT_VERTEX;
                g->grid[column][row].arcs[0] = VACANT_ARC;
                g->grid[column][row].arcs[1] = VACANT_ARC;
                g->grid[column][row].arcs[2] = VACANT_ARC;
            }
            column++;
        }
        column = 0;
        row++;
    }

    // holds which uni currently has the most ARCs
    g->uniWithMostARCs = NO_ONE;
    g->uniWithMostARCs_number = NO_ONE;

    // holds which uni currently has the most publications
    g->uniWithMostPubs = NO_ONE;
    g->uniWithMostPubs_number = NO_ONE;

    // create the campuses
    g->grid[3][0].vertices[1] = CAMPUS_A;
    g->grid[3][5].vertices[0] = CAMPUS_A;
    g->grid[1][2].vertices[0] = CAMPUS_C;
    g->grid[5][3].vertices[1] = CAMPUS_C;
    g->grid[0][5].vertices[1] = CAMPUS_B;
    g->grid[6][0].vertices[0] = CAMPUS_B;

    // create the retrainers
    g->grid[2][5].retrainCenters[0] = STUDENT_MTV;
    g->grid[2][5].retrainCenters[1] = STUDENT_MTV;
    g->grid[4][4].retrainCenters[0] = STUDENT_MMONEY;
    g->grid[4][4].retrainCenters[1] = STUDENT_MMONEY;
    g->grid[6][1].retrainCenters[0] = STUDENT_BQN;
    g->grid[5][1].retrainCenters[1] = STUDENT_BQN;
    g->grid[5][0].retrainCenters[0] = STUDENT_MJ;
    g->grid[4][0].retrainCenters[1] = STUDENT_MJ;
    g->grid[1][2].retrainCenters[1] = STUDENT_BPS;
    g->grid[2][1].retrainCenters[0] = STUDENT_BPS;

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

    int player = getWhoseTurn(g);

    int playerCampus;
    int playerGroupOfEight;
    int playerArc;

    if(player == 1) {
        playerCampus = CAMPUS_A;
        playerGroupOfEight = GO8_A;
        playerArc = ARC_A;
    } else if (player == 2) {
        playerCampus = CAMPUS_B;
        playerGroupOfEight = GO8_B;
        playerArc = ARC_B;
    } else {
        playerCampus = CAMPUS_C;
        playerGroupOfEight = GO8_C;
        playerArc = ARC_C;
    }

    coord locateV = pathToVertex(a.destination);
    coord locateA = pathToARC(a.destination);

    if(a.actionCode == BUILD_CAMPUS) {
        g->grid[locateV.x][locateV.y].vertices[locateV.vertNum]
            = playerCampus;
        g->numCampuses[player-1]++;
        g->studentAmounts[player-1][STUDENT_BPS]--;
        g->studentAmounts[player-1][STUDENT_BQN]--;
        g->studentAmounts[player-1][STUDENT_MJ]--;
        g->studentAmounts[player-1][STUDENT_MTV]--;
        g->numKPI[player-1] += CAMPUS_KPI;
    } else if (a.actionCode == BUILD_GO8) {
        g->grid[locateV.x][locateV.y].vertices[locateV.vertNum]
            = playerGroupOfEight;
        g->numGO8s[player-1]++;
        g->numCampuses[player-1]--;
        g->studentAmounts[player-1][STUDENT_MJ] -= 2;
        g->studentAmounts[player-1][STUDENT_MMONEY] -= 3;

        // total increase in KPI is 10 since we lose
        // one campus (10 KPI) to gain a GO8 (20 KPI)
        g->numKPI[player-1] -= CAMPUS_KPI;
        g->numKPI[player-1] += GO8_KPI;
    } else if (a.actionCode == OBTAIN_ARC) {
        g->grid[locateA.x][locateA.y].arcs[locateA.arcNum]
            = playerArc;
        g->numARCs[player-1]++;
        g->studentAmounts[player-1][STUDENT_BPS]--;
        g->studentAmounts[player-1][STUDENT_BQN]--;
        g->numKPI[player-1] += ARC_KPI;

        // checks for prestige bonus regarding having most ARC grants
        if (g->uniWithMostARCs == player) {
            g->uniWithMostARCs_number++;
        }
        if(g->numARCs[player-1] > g->uniWithMostARCs_number) {
            if(getMostARCs(g) != NO_ONE &&
               getMostARCs(g) != player){
                g->numKPI[getMostARCs(g)-1] -= PRESTIGE_BONUS;
            }
            g->numKPI[player-1] += PRESTIGE_BONUS;
            g->uniWithMostARCs_number = g->numARCs[player-1];
            g->uniWithMostARCs = player;
        }
    } else if (a.actionCode == OBTAIN_PUBLICATION) {
        g->studentAmounts[player-1][STUDENT_MJ]--;
        g->studentAmounts[player-1][STUDENT_MTV]--;
        g->studentAmounts[player-1][STUDENT_MMONEY]--;
        g->numPubs[player-1]++;

        // checks for prestige bonus regarding having most publications
        if(g->numPubs[player-1] > g->uniWithMostPubs_number) {
            if(getMostPublications(g) != NO_ONE &&
               getMostPublications(g) != player) {
                g->numKPI[getMostPublications(g)-1] -= PRESTIGE_BONUS;
            }
            g->numKPI[player-1] += PRESTIGE_BONUS;
            g->uniWithMostPubs_number = g->numPubs[player-1];
            g->uniWithMostPubs = player;
        }
    } else if (a.actionCode == OBTAIN_IP_PATENT) {
        g->studentAmounts[player-1][STUDENT_MJ]--;
        g->studentAmounts[player-1][STUDENT_MTV]--;
        g->studentAmounts[player-1][STUDENT_MMONEY]--;
        g->numIPs[player-1]++;
        g->numKPI[player-1] += IP_KPI;
    } else if (a.actionCode == RETRAIN_STUDENTS) {
        int exchangeRate = getExchangeRate(g, player,
            a.disciplineFrom, a.disciplineTo);
        g->studentAmounts[player-1][a.disciplineFrom] -= exchangeRate;
        g->studentAmounts[player-1][a.disciplineTo]++;
    } 

}


// advance the game to the next turn, 
// assuming that the dice has just been rolled and produced diceScore
// the game starts in turn -1 (we call this state "Terra Nullis") and 
// moves to turn 0 as soon as the first dice is thrown. 
void throwDice (Game g, int diceScore) {
   g->turnNumber++;
   int X = 0;
   int Y = 0;
   int pCount = 0;
   int add = 0;

   // Check the entire board for the hex with the equivalent diceScore
   while (Y < GRID_HEIGHT){
      X = 0;
      while (X < GRID_WIDTH){
         if (g->grid[X][Y].diceNum == diceScore){
            // make 3 passes, one for each player
            pCount = 0;
            while (pCount < 3){
               add = 0;
               // Search adjacent vertexes and increment acccordingly
               if (g->grid[X][Y].vertices[0] == pCount + 1){
                  add++;
               }
               if (g->grid[X][Y].vertices[1] == pCount + 1){
                  add++;
               }
               if (g->grid[X][Y-1].vertices[0] == pCount + 1){
                  add++;
               }
               if (g->grid[X][Y-1].vertices[1] == pCount + 1){
                  add++;
               }
               if (g->grid[X-1][Y].vertices[1] == pCount + 1){
                  add++;
               }
               if (g->grid[X+1][Y-1].vertices[0] == pCount + 1){
                  add++;
               }
               if (g->grid[X][Y].vertices[0] == pCount + 4){
                  add+=2;
               }
               if (g->grid[X][Y].vertices[1] == pCount + 4){
                  add+=2;
               }
               if (g->grid[X][Y-1].vertices[0] == pCount + 4){
                  add+=2;
               }
               if (g->grid[X][Y-1].vertices[1] == pCount + 4){
                  add+=2;
               }
               if (g->grid[X-1][Y].vertices[1] == pCount + 4){
                   add+=2;
               }
               if (g->grid[X+1][Y-1].vertices[0] == pCount + 4){
                  add+=2;
               }

               // Adds the accumulated students for that pass
               g->studentAmounts[pCount][g->grid[X][Y].resType] += add;
               pCount++;
            }//endwhile
         }//endif
         X++;
      }//endwhile
      Y++;
   }//endwhile

   if (diceScore == 7) {
      int player = 0;
      while (player < NUM_UNIS) {
         g->studentAmounts[player][STUDENT_THD] += 
            g->studentAmounts[player][STUDENT_MTV]
               + g->studentAmounts[player][STUDENT_MMONEY];
         g->studentAmounts[player][STUDENT_MTV] = 0;
         g->studentAmounts[player][STUDENT_MMONEY] = 0;
         player++;
     }
   }
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
int getCampus(Game g, path inPath) {
     //get the 2d coord of the path
     coord vertex = pathToVertex(inPath);
     return g->grid[vertex.x][vertex.y].vertices[vertex.vertNum];
}


// return the contents of the given edge (ie ARC code or vacant ARC)
int getARC(Game g, path pathToEdge) {
    //get the 2d coord of the path
    coord arc = pathToARC(pathToEdge);
    return g->grid[arc.x][arc.y].arcs[arc.arcNum];
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

    // it is illegal to use a path that leaves the board
    if (a.actionCode == OBTAIN_ARC || a.actionCode == BUILD_GO8
            || a.actionCode == BUILD_CAMPUS) {
        if (isPathContained(a.destination) == FALSE) {
            isLegal = FALSE;
        }
    }

    // it is illegal to make any action during terra nullis
    if (getTurnNumber(g) == -1) {
        isLegal = FALSE;
    }
    
    // check that conditions are satisfied for building a campus
    if (a.actionCode == BUILD_CAMPUS) {
        int resourceA = getStudents(g, getWhoseTurn(g), STUDENT_BPS);
        int resourceB = getStudents(g, getWhoseTurn(g), STUDENT_BQN);
        int resourceC = getStudents(g, getWhoseTurn(g), STUDENT_MJ);
        int resourceD = getStudents(g, getWhoseTurn(g), STUDENT_MTV);
        
        //check for enough students
        if (resourceA<1 || resourceB<1 || resourceC<1 || resourceD<1) {
            isLegal = FALSE;
        }

        //check if the vertex is vacant and there is an arc
        if (getCampus(g,a.destination) != VACANT_VERTEX) {
            isLegal = FALSE;
        }

        // check the campus is connected to one of the player's ARCs
        if (isCampusConnected(a.destination, g, getWhoseTurn(g)) 
                == FALSE) {
            isLegal = FALSE;
        }

        // is the vertex we are building at adjacent to another campus
        if (isCampusTooClose(g, a.destination) == TRUE) {
            isLegal = FALSE;
        }
    }

    // check that conditions are satisfied for building a GO8
    if (a.actionCode == BUILD_GO8) {
        int resourceA = getStudents(g, getWhoseTurn(g), STUDENT_MJ);
        int resourceB = getStudents(g, getWhoseTurn(g), STUDENT_MMONEY);

        // check for enough students 
        if (resourceA<2 || resourceB<3) {
            isLegal = FALSE;
        }

        // check player has a campus there already
        if (getCampus(g, a.destination) != getWhoseTurn(g)) {
            isLegal = FALSE;
        }
    }

    // check that conditions are satisfied for building an ARC
    if (a.actionCode == OBTAIN_ARC) {
        int resourceA = getStudents(g, getWhoseTurn(g), STUDENT_BPS);
        int resourceB = getStudents(g, getWhoseTurn(g), STUDENT_BQN);

        // check for enough students
        if (resourceA<1 || resourceB<1) {
            isLegal = FALSE;
        }

        // check the building destination is empty
        if (getARC(g,a.destination) != VACANT_ARC) {
            isLegal = FALSE;
        }

        // check the player has connected arcs/campuses
        if (isARCConnected(a.destination, g, getWhoseTurn(g)) == FALSE) {
            isLegal = FALSE;
        }
    }
    
    
    // check there are enough students to start a spinoff
    if (a.actionCode == START_SPINOFF) {
        int resourceA = getStudents(g, getWhoseTurn(g), STUDENT_MJ);
        int resourceB = getStudents(g, getWhoseTurn(g), STUDENT_MTV);
        int resourceC = getStudents(g, getWhoseTurn(g), STUDENT_MMONEY);
        if (resourceA < 1 || resourceB < 1 || resourceC < 1) {
            isLegal = FALSE;
        }
    }
    

    // check there are enough students to retrain
    if (a.actionCode == RETRAIN_STUDENTS) {
        int resourceA = getStudents(g, getWhoseTurn(g), a.disciplineFrom);
        int resourceB = getStudents(g, getWhoseTurn(g), a.disciplineTo);
        
        // check if the user entered a valid student type
        if ((resourceA > 5 && resourceA < 1) 
                || (resourceB > 5 && resourceB < 0)) {
            isLegal = FALSE;
        }
        
        // check for enough of the from student to retrain
        if (getStudents(g, getWhoseTurn(g), a.disciplineFrom)
                < getExchangeRate(g, getWhoseTurn(g), a.disciplineFrom,
                    a.disciplineTo)) {
            isLegal = FALSE;
        }
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

    // checks that the discipline that
    // wants to be retrained is not a THD
    assert(disciplineFrom != STUDENT_THD);

    // checks that the discipline that
    // wants to be trained into is valid
    assert((disciplineTo == STUDENT_THD)  ||
           (disciplineTo == STUDENT_BPS)  ||
           (disciplineTo == STUDENT_BQN)  ||
           (disciplineTo == STUDENT_MJ )  ||
           (disciplineTo == STUDENT_MTV)  ||
           (disciplineTo == STUDENT_MMONEY));

    int playerCampus;
    int playerGroupOfEight;

    if (player == UNI_A) {
        playerCampus = CAMPUS_A;
        playerGroupOfEight = GO8_A;
    } else if (player == UNI_B) {
        playerCampus = CAMPUS_B;
        playerGroupOfEight = GO8_B;
    } else {
        playerCampus = CAMPUS_C;
        playerGroupOfEight = GO8_C;
    }
    // by default, exchange rate is 3. If a player's campus lies
    // on a retraining centre, the exchange rate to retrain a
    // discipline (identical to the type of retraining centre)
    // falls to 2.
    int exchangeRate = DEFAULT_EXCHANGE_RATE;
    int x = 0;
    int y = 0;
    int z = 0;

    while(y < GRID_HEIGHT) {
        while(x < GRID_WIDTH) {
            while(z < NUM_VERTICES_PER_HEX){
                if(disciplineFrom == STUDENT_MTV) {
                    if(playerCampus == 
                       g->grid[2][5].vertices[0] ||
                       playerCampus ==
                       g->grid[2][5].vertices[1] ||
                       playerGroupOfEight ==
                       g->grid[2][5].vertices[0] ||
                       playerGroupOfEight ==
                       g->grid[2][5].vertices[1]) {
                        exchangeRate = DISCOUNT_EXCHANGE_RATE;
                        x = GRID_WIDTH;
                        y = GRID_HEIGHT;
                        z = NUM_VERTICES_PER_HEX;                        
                    }
                } else if(disciplineFrom == STUDENT_MMONEY) {
                    if(playerCampus ==
                       g->grid[4][4].vertices[0] ||
                       playerCampus ==
                       g->grid[4][4].vertices[1] ||
                       playerGroupOfEight ==
                       g->grid[4][4].vertices[0] ||
                       playerGroupOfEight ==
                       g->grid[4][4].vertices[1]) {
                        exchangeRate = DISCOUNT_EXCHANGE_RATE;
                        x = GRID_WIDTH;
                        y = GRID_HEIGHT;
                        z = NUM_VERTICES_PER_HEX; 
                    }
                } else if(disciplineFrom == STUDENT_BQN) {
                    if(playerCampus ==
                       g->grid[6][1].vertices[0] ||
                       playerCampus ==
                       g->grid[5][1].vertices[1] ||
                       playerGroupOfEight ==
                       g->grid[6][1].vertices[0] ||
                       playerGroupOfEight ==
                       g->grid[5][1].vertices[1]) {
                        exchangeRate = DISCOUNT_EXCHANGE_RATE;
                        x = GRID_WIDTH;
                        y = GRID_HEIGHT;
                        z = NUM_VERTICES_PER_HEX; 
                    }
                } else if(disciplineFrom == STUDENT_MJ) {
                    if(playerCampus ==
                       g->grid[5][0].vertices[0] ||
                       playerCampus ==
                       g->grid[4][0].vertices[1] ||
                       playerGroupOfEight ==
                       g->grid[5][0].vertices[0] ||
                       playerGroupOfEight ==
                       g->grid[4][0].vertices[1]) {
                        exchangeRate = DISCOUNT_EXCHANGE_RATE;
                        x = GRID_WIDTH;
                        y = GRID_HEIGHT;
                        z = NUM_VERTICES_PER_HEX; 
                    }
                } else if(disciplineFrom == STUDENT_BPS) {
                    if(playerCampus ==
                       g->grid[1][2].vertices[1] ||
                       playerCampus ==
                       g->grid[2][1].vertices[0] ||
                       playerGroupOfEight ==
                       g->grid[1][2].vertices[1] ||
                       playerGroupOfEight ==
                       g->grid[2][1].vertices[0]) {
                        exchangeRate = DISCOUNT_EXCHANGE_RATE;
                        x = GRID_WIDTH;
                        y = GRID_HEIGHT;
                        z = NUM_VERTICES_PER_HEX; 
                    }
                }
                z++;
            }
            x++;
            z = 0;
        }
        y++;
        x = 0;
    }

    return exchangeRate;
}
