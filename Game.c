#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "Game.h"


#define GRID_WIDTH 7
#define GRID_HEIGHT 6
#define NUM_ARCS_PER_HEX 3
#define NUM_VERTICES_PER_HEX 2
#define NUM_RETRAINS_PER_HEX 2
#define NUM_DISCIPLINES 6
#define OUTSIDE_BOARD -1


// Represents a single coordinate within the 2d grid.
// Simplifies the process of referring to a particular arc/vertex within
// the grid, since we can give the exact x/y coordinates plus which 
// arc/vertex in the hex we require
typedef struct _coord {
    int x;
    int y;
    int arcNum;
    int vertNum;
} coord;


// functions to convert between the regionID and our coordinate system
static coord regIDToCoord(int regID);
static int coordToRegID(coord inCoord);

// functions to return which arc (or vertex) is at the end of a path
static coord pathToARC(path inPath);
static coord pathToVertex(path inPath);

// function which traverses the path to make sure it is contained 
static int isPathContained(path inPath);


// used throughout the implementation, stores a particular location in
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


// a struct that represents the board
typedef struct _game {
    // a 7 wide by 6 high grid of the board, each element is type hex
    // grid[x][y]
    hex grid[GRID_WIDTH][GRID_HEIGHT];

    // what turn we are currently on
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

    // holds which uni currently has the most arcs
    int uniWithMostARCs;
    int uniWithMostARCs_number;

    // holds which uni currently has the most publications
    int uniWithMostPubs;
    int uniWithMostPubs_number;
} game;


// return the coordinate of a hex given its region ID
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


// return the region ID of a particular coordinate (-1 if outside board)
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
