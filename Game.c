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
