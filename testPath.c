#include<stdio.h>
#include<stdlib.h>
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

typedef struct _coord {
    int x;
    int y;
    int arcNum;
    int vertNum;
} coord;

typedef struct _hex {
    int resType;
    int diceNum;
    int arcs[NUM_ARCS_PER_HEX];
    int vertices[NUM_VERTICES_PER_HEX];
    int retrainCenters[NUM_RETRAINS_PER_HEX];
} hex;

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

//tests used for PathToVertex and PathToArc
int main(int argc, char *argv[]){
    puts("testing path to vertex......");
    coord test;

    test = pathToVertex("L");
    assert(test.x == 3);
    assert(test.y == 5);
    assert(test.arcNum == -1);
    assert(test.vertNum == 1);

    test = pathToVertex("LB");
    assert(test.x == 3);
    assert(test.y == 5);
    assert(test.arcNum == -1);
    assert(test.vertNum == 0);

    test = pathToVertex("LBLLRB");
    assert(test.x == 3);
    assert(test.y == 4);
    assert(test.arcNum == -1);
    assert(test.vertNum == 0);

    test = pathToVertex("LR");
    assert(test.x == 4);
    assert(test.y == 4);
    assert(test.arcNum == -1);
    assert(test.vertNum == 0);

    test = pathToVertex("RLR");
    assert(test.x == 2);
    assert(test.y == 4);
    assert(test.arcNum == -1);
    assert(test.vertNum == 1);

    test = pathToVertex("RLRRLRR");
    assert(test.x == 0);
    assert(test.y == 5);
    assert(test.arcNum == -1);
    assert(test.vertNum == 1);

    test = pathToVertex("RLRL");
    assert(test.x == 3);
    assert(test.y == 3);
    assert(test.arcNum == -1);
    assert(test.vertNum == 0);

    test = pathToVertex("RLRLLRRL");
    assert(test.x == 4);
    assert(test.y == 1);
    assert(test.arcNum == -1);
    assert(test.vertNum == 0);

    test = pathToVertex("RLRLLRRLLR");
    assert(test.x == 5);
    assert(test.y == 0);
    assert(test.arcNum == -1);
    assert(test.vertNum == 0);

    //this one goes outside the board and back
    test = pathToVertex("RLRRLRRLLLL");
    assert(test.x == 0);
    assert(test.y == 4);
    assert(test.arcNum == -1);
    assert(test.vertNum == 1);

    //this is an invalid input
    test = pathToVertex("AAA");
    assert(test.x == 3);
    assert(test.y == 5);
    assert(test.arcNum == -1);
    assert(test.vertNum == 0);

    printf("all test passed\n");
}

