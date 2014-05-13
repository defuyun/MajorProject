#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Game.c"

#define GRID_WIDTH 7
#define GRID_HEIGHT 6
#define NUM_ARCS_PER_HEX 3
#define NUM_VERTICES_PER_HEX 2
#define NUM_RETRAINS_PER_HEX 2
#define NUM_DISCIPLINES 6
#define OUTSIDE_BOARD -1

#define DEFAULT_DISCIPLINES { \
    STUDENT_BQN,    STUDENT_MMONEY, STUDENT_MJ, \
    STUDENT_MMONEY, STUDENT_MJ,     STUDENT_BPS, \
    STUDENT_MTV,    STUDENT_MTV,    STUDENT_BPS, \
    STUDENT_MTV,    STUDENT_BQN,    STUDENT_MJ, \
    STUDENT_BQN,    STUDENT_THD,    STUDENT_MJ, \
    STUDENT_MMONEY, STUDENT_MTV,    STUDENT_BQN, \
    STUDENT_BPS }

#define DEFAULT_DICE { \
    9, 10,  8, 12,  6,  5,  \
    3, 11,  3, 11,  4,  6, \
    4,  9,  9,  2,  8, 10, \
    5 }

// function which steps through the path, beginning at the default
// starting point on the board, and if the path leaves the board at any 
// time, returns FALSE. If the path remains contained within the board
// it returns TRUE. "RRRRRR" is not contained, whereas "LRRRRR" is.
static int isPathContained(path inPath);

// when building a new campus, call "isCampusConnected" on the 
// destination vertex to ensure there are arcs adjacent. When building 
// an ARC, call isARCConnected on the destination edge to ensure an
// ARC or campus is adjacent
// static int isARCConnected(path inPath, int player);
// static int isCampusConnected(path inPath, int player);

// returns true if there are campuses next to the vertex at inPath
static int isCampusTooClose(Game g, path inPath);

int main (int argc, char *argv[]) {

    // Assert that regions are being converted to and from coordinates 
    // correctly by stepping through each region one at a time.
    puts("Testing coordToRegID() and regIDToCoord()");
    int i = 0;
    while (i < NUM_REGIONS) {
        assert(coordToRegID(regIDToCoord(i)) == i);
        i++;
    }

    // Check that we can detect whether a coordinate is outside the 
    // board or not
    puts("Testing isCoordInside()");
    coord c = {.x = 3, .y = 1, .arcNum = 1, .vertNum = -1};
    assert(isCoordInside(c) == TRUE);
    c.y--;
    assert(isCoordInside(c) == TRUE);
    c.x--;
    assert(isCoordInside(c) == FALSE);
    c.y++;
    assert(isCoordInside(c) == TRUE);
    c.x = 6;
    c.y = 1;
    c.arcNum = -1;
    c.vertNum = 0;
    assert(isCoordInside(c) == TRUE);
    coord leftEdge = {.x = 0, .y = 4, .arcNum = 2, .vertNum = -1};
    assert(isCoordInside(leftEdge) == TRUE);
    leftEdge.arcNum = 1;
    assert(isCoordInside(leftEdge) == FALSE);
    leftEdge.vertNum = 1;
    leftEdge.arcNum = -1;
    assert(isCoordInside(leftEdge) == TRUE);
    leftEdge.vertNum = 0;
    assert(isCoordInside(leftEdge) == FALSE);
    leftEdge.x = 6;
    leftEdge.y = 0;
    leftEdge.arcNum = 1;
    leftEdge.vertNum = -1;
    assert(isCoordInside(leftEdge) == FALSE);

    assert(isPathContained("RLRRLRR") == TRUE);
    assert(isPathContained("LRLRLRRLRLL") == FALSE);

    // check that pathToVertex works properly
    puts("testing pathToVertex()......");
    coord test;
    puts("test 1....");
    test = pathToVertex("L");
    assert(test.x == 3);
    assert(test.y == 5);
    assert(test.arcNum == -1);
    assert(test.vertNum == 1);
    
    puts("test 2....");
    test = pathToVertex("LB");
    assert(test.x == 3);
    assert(test.y == 5);
    assert(test.arcNum == -1);
    assert(test.vertNum == 0);

    puts("test 3....");
    test = pathToVertex("LBLLRB");
    assert(test.x == 3);
    assert(test.y == 4);
    assert(test.arcNum == -1);
    assert(test.vertNum == 0);

    puts("test 4....");
    test = pathToVertex("LR");
    assert(test.x == 4);
    assert(test.y == 4);
    assert(test.arcNum == -1);
    assert(test.vertNum == 0);

    puts("test 5....");
    test = pathToVertex("RLR");
    assert(test.x == 2);
    assert(test.y == 4);
    assert(test.arcNum == -1);
    assert(test.vertNum == 1);

    puts("test 6....");
    test = pathToVertex("RLRRLRR");
    assert(test.x == 0);
    assert(test.y == 5);
    assert(test.arcNum == -1);
    assert(test.vertNum == 1);

    puts("test 7....");
    test = pathToVertex("RLRL");
    assert(test.x == 3);
    assert(test.y == 3);
    assert(test.arcNum == -1);
    assert(test.vertNum == 0);

    puts("test 8....");
    test = pathToVertex("RLRLLRRL");
    assert(test.x == 4);
    assert(test.y == 1);
    assert(test.arcNum == -1);
    assert(test.vertNum == 0);

    puts("test 9....");
    test = pathToVertex("RLRLLRRLLR");
    assert(test.x == 5);
    assert(test.y == 0);
    assert(test.arcNum == -1);
    assert(test.vertNum == 0);

    puts("test 10....");
    //this one goes outside the board and back
    test = pathToVertex("RLRRLRRLLLL");
    assert(test.x == 0);
    assert(test.y == 4);
    assert(test.arcNum == -1);
    assert(test.vertNum == 1);

    // check that pathToARC works properly
    puts("testing PathToArc()....");

    puts("test 1....");
    test = pathToARC("L");
    assert(test.x == 3);
    assert(test.y == 5);
    assert(test.arcNum == 1);
    assert(test.vertNum == -1);

    puts("test 2....");
    test = pathToARC("LB");
    assert(test.x == 3);
    assert(test.y == 5);
    assert(test.arcNum == 1);
    assert(test.vertNum == -1);

    puts("test 3....");
    test = pathToARC("LBLLRB");
    assert(test.x == 3);
    assert(test.y == 4);
    assert(test.arcNum == 0);
    assert(test.vertNum == -1);

    puts("test 4....");
    test = pathToARC("LR");
    assert(test.x == 3);
    assert(test.y == 5);
    assert(test.arcNum == 2);
    assert(test.vertNum == -1);

    puts("test 5....");
    test = pathToARC("RLR");
    assert(test.x == 3);
    assert(test.y == 4);
    assert(test.arcNum == 0);
    assert(test.vertNum == -1);

    puts("test 6....");
    test = pathToARC("RLRRLRR");
    assert(test.x == 0);
    assert(test.y == 5);
    assert(test.arcNum == 2);
    assert(test.vertNum == -1);

    puts("test 7....");
    test = pathToARC("RLRL");
    assert(test.x == 2);
    assert(test.y == 4);
    assert(test.arcNum == 2);
    assert(test.vertNum == -1);

    puts("test 8....");
    test = pathToARC("RLRLLRRL");
    assert(test.x == 3);
    assert(test.y == 2);
    assert(test.arcNum == 2);
    assert(test.vertNum == -1);

    puts("test 9....");
    test = pathToARC("RLRLLRRLLR");
    assert(test.x == 4);
    assert(test.y == 1);
    assert(test.arcNum == 2);
    assert(test.vertNum == -1);


    // check that isCampusTooClose works correctly
    puts("Testing isCampusTooClose()");
    int disciplines[] = DEFAULT_DISCIPLINES;
    int dice[] = DEFAULT_DICE;
    Game g = newGame(disciplines, dice);
    assert(isCampusTooClose(g, "R") == TRUE);
    assert(isCampusTooClose(g, "L") == TRUE);
    assert(isCampusTooClose(g, "RR") == FALSE);
    assert(isCampusTooClose(g, "LR") == FALSE);
    assert(isCampusTooClose(g, "RRLR") == TRUE);


    puts("All tests for static functions passed!\n");

    return EXIT_SUCCESS;
}
