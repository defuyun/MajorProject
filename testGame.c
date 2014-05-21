#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Game.h"


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

#define RESOURCE_DISCIPLINES { \
    STUDENT_BQN,    STUDENT_MMONEY, STUDENT_MJ,  \
    STUDENT_MMONEY, STUDENT_MJ,     STUDENT_BPS,  \
    STUDENT_MTV,    STUDENT_BQN,    STUDENT_BPS,  \
    STUDENT_MTV,    STUDENT_BQN,    STUDENT_BPS,  \
    STUDENT_BQN,    STUDENT_THD,    STUDENT_MJ,  \
    STUDENT_MMONEY, STUDENT_MTV,    STUDENT_BQN,  \
    STUDENT_BPS }

#define RESOURCE_DICE { \
    9, 10,  8, 12,  6,  5, \
    3,  2,  3, 11,  4,  2, \
    4,  7,  9,  2,  8, 10, \
    5 }

#define INITIAL_TURN         (-1)
#define NUM_DISCIPLINES      (6)
#define MIN_DICE_VAL         (1)
#define MAX_DICE_VAL         (6)
#define NUM_DICE             (2)
#define NUM_TURNS_TO_TEST    (100)
#define NUM_INITIAL_CAMPUSES (2)

#define INITIAL_KPI          (20)
#define INITIAL_ARCS         (0)
#define INITIAL_GO8          (0)
#define INITIAL_IP           (0)
#define INITIAL_PUBLICATIONS (0)

#define INITIAL_THD          (0)
#define INITIAL_BPS          (3)
#define INITIAL_BQN          (3)
#define INITIAL_MJ           (1)
#define INITIAL_MTV          (1)
#define INITIAL_MMONEY       (1)

#define INITIAL_STUDENTS_NUM { \
    INITIAL_THD,    INITIAL_BPS,    INITIAL_BQN, \
    INITIAL_MJ,     INITIAL_MTV ,   INITIAL_MMONEY }


// run the test suite
void beginTesting(void);


void testNewGame(void);             // MATTHEW
void testMakeAction(void);          // JAMES 
void testThrowDice(void);           // TIM
void testGetDiscipline(void);       // MATTHEW
void testGetDiceValue(void);        // MATTHEW
void testGetMostARCS(void);         // CARL
void testGetMostPublications(void); // JAMES
void testGetTurnNumber(void);       // MATTHEW
void testGetWhoseTurn(void);        // TIM
void testGetCampus(void);           // CARL
void testGetARC(void);              // MATTHEW
void testIsLegalAction(void);       // JAMES
void testGetKPIpoints(void);        // TIM
void testGetARCs(void);             // CARL
void testGetGO8s(void);             // JAMES 
void testGetCampuses(void);         // TIM
void testGetIPs(void);              // CARL
void testGetPublications(void);     // JAMES
void testGetStudents(void);         // TIM
void testGetExchangeRate(void);     // CARL


// helper functions to assist with testing
void genResources(Game g, int count, int diceNum);
void buildARC(Game g, char *myPath);
void buildCampus(Game g, char *myPath);
void buildGO8(Game g, char *myPath);
void retrain(Game g, int fromStudent, int toStudent, int count);
void getPub(Game g, int count);
void checkStudents(Game g, int bps, int bqn, int mj, int mtv,
        int mmoney, int thd);
void checkIPs(Game g, int uniAIp, int uniBIp, int uniCIp);
void checkPubs(Game g, int uniAPubs, int uniBPubs, int uniCPubs);
void checkGO8s(Game g, int uniAGO8, int uniBGO8, int uniCGO8);
void checkCampuses(Game g, int uniACmp, int uniBCmp, int uniCCmp);
void runGame(Game g);
void endTurn(Game g);


int main(int argc, char *argv[]) {
    beginTesting();
    return EXIT_SUCCESS;
}


// run the suite of tests from start to finish
void beginTesting(void) {
    puts("Initialising test sequence...");

    testNewGame();
    testMakeAction();
    testThrowDice();
    testGetDiscipline();
    testGetDiceValue();
    testGetMostARCS();
    testGetMostPublications();
    testGetTurnNumber();
    testGetWhoseTurn();
    testGetCampus();
    testGetARC();
    testGetKPIpoints();
    testGetARCs();
    testGetGO8s();
    testGetCampuses();
    testGetIPs();
    testGetPublications();
    testGetExchangeRate();
    testIsLegalAction();
    testGetStudents();

    puts("Congrats, testing found no errors!");
}


// advance the game to the next turn, 
// assuming that the dice has just been rolled and produced diceScore
// the game starts in turn -1 (we call this state "Terra Nullis") and 
// moves to turn 0 as soon as the first dice is thrown. 
void testThrowDice(void) {
    puts("Testing function throwDice()...");
    int setResource[] = DEFAULT_DISCIPLINES;
    int setDice[] = DEFAULT_DICE;

    Game A = newGame(setResource, setDice);

    // Checking initial state
    assert(getTurnNumber(A) == -1);
    assert(getWhoseTurn(A) == NO_ONE);

    // Stress testing throwDice for correct turn assignment
    int count = 0;
    while (count < 500){
      throwDice(A,7);
      assert(getTurnNumber(A) == count);
      count++;
    }
    disposeGame(A);

    Game B = newGame(setResource, setDice);
    assert(getStudents(B, UNI_A,STUDENT_MJ) == 1);
    count = 0;
    while (count < 15){
      throwDice(B,6);
      assert(getStudents(B, UNI_A,STUDENT_MJ) == 2 + count);
      count++;
    }

    disposeGame(B);

}


// return the player id of the player whose turn it is
// the result of this function is NO_ONE during Terra Nullis
void testGetWhoseTurn(void) {
    puts("Testing function getWhoseTurn()...");
    int setResource[] = DEFAULT_DISCIPLINES;
    int setDice[] = DEFAULT_DICE;

    Game A = newGame(setResource, setDice);
    assert(getWhoseTurn(A) == NO_ONE);
    int count = 0;


    // Checking the turn order across a 1500 turn (500 round) game
    while (count < 1500){
      throwDice(A,7);
      assert(getWhoseTurn(A) == 1 + (count % 3));
      count++;
    }

    disposeGame(A);

}


// return the number of KPI points the specified player currently has
void testGetKPIpoints(void) {
    puts("Testing function getKPIpoints()...");
    int setResource[] = DEFAULT_DISCIPLINES;
    int setDice[] = DEFAULT_DICE;

    Game A = newGame(setResource, setDice);

    // Testing initial state

    assert(getKPIpoints(A,UNI_A) == 20);
    assert(getKPIpoints(A,UNI_B) == 20);
    assert(getKPIpoints(A,UNI_C) == 20);

    runGame(A);

    // A Uni turn
    buildARC(A, "L");

    // 20 + 2 from arc + 10 from prestige
    assert(getKPIpoints(A,UNI_A) == 32);
    buildARC(A, "LR");
    assert(getKPIpoints(A,UNI_A) == 34);
    buildARC(A, "LRR");
    assert(getKPIpoints(A,UNI_A) == 36);
    buildARC(A, "LRRL");
    assert(getKPIpoints(A,UNI_A) == 38);

    buildCampus(A, "LR");
    assert(getKPIpoints(A,UNI_A) == 48);
    buildCampus(A, "LRRL");
    assert(getKPIpoints(A,UNI_A) == 58);

    buildGO8(A, "LR");
    assert(getKPIpoints(A,UNI_A) == 68);
    buildGO8(A, "LRRL");
    assert(getKPIpoints(A,UNI_A) == 78);

    endTurn(A);

    // B Uni turn
    buildARC(A, "RRLRL");

    // 20 + 2 for ARCs. Uni A has the prestige points for ARCs
    assert(getKPIpoints(A,UNI_B) == 22);
    buildARC(A, "RRLR");
    assert(getKPIpoints(A,UNI_B) == 24);
    buildARC(A, "RRLL");
    assert(getKPIpoints(A,UNI_B) == 26);
    buildARC(A, "RRLLL");
    assert(getKPIpoints(A,UNI_B) == 28);

    // 28 + 10 for campuses
    buildCampus(A, "RRL");
    assert(getKPIpoints(A,UNI_B) == 38);
    buildCampus(A, "RRLLL");
    assert(getKPIpoints(A,UNI_B) == 48);

    // 48 - 10 + 20 from conversion of campus to GO8
    buildGO8(A, "RRL");
    assert(getKPIpoints(A,UNI_B) == 58);
    buildGO8(A, "RRLLL");
    assert(getKPIpoints(A,UNI_B) == 68);

    // Checking the transfer of prestige from A to B
    buildARC(A, "RRLLR");
    // 68 + 2 from arc + 10 from prestige
    assert(getKPIpoints(A,UNI_B) == 80);
    // 78 - 10 from loss of prestige
    assert(getKPIpoints(A,UNI_A) == 68);

    endTurn(A);

    // C Uni turn
    buildARC(A, "LRLRLR");
    // 20 + 2 from ARC
    assert(getKPIpoints(A,UNI_C) == 22);
    buildARC(A, "LRLRLRR");
    assert(getKPIpoints(A,UNI_C) == 24);
    buildARC(A, "LRLRLRRR");
    assert(getKPIpoints(A,UNI_C) == 26);
    buildARC(A, "LRLRLRRRL");
    assert(getKPIpoints(A,UNI_C) == 28);

    // 28 + 10 from campus
    buildCampus(A, "LRLRLRR");
    assert(getKPIpoints(A,UNI_C) == 38);
    buildCampus(A, "LRLRLRRRL");
    assert(getKPIpoints(A,UNI_C) == 48);

    // 48 - 10 from campus + 20 from GO8
    buildGO8(A, "LRLRLRR");
    assert(getKPIpoints(A,UNI_C) == 58);
    buildGO8(A, "LRLRLRRRL");
    assert(getKPIpoints(A,UNI_C) == 68);

    // Check that prestige can be reobtained
    buildARC(A, "LRLRLRRRR");
    buildARC(A, "LRLRLRRRLR");
    // 68 + 2(a) + 2(a) + 10(p)
    assert(getKPIpoints(A,UNI_C) == 82);
    // 80 - 10(p) for Uni B
    assert(getKPIpoints(A,UNI_B) == 70);
    endTurn(A);

    buildARC(A, "R");
    buildARC(A, "RR");
    buildARC(A, "RL");
    // 68 + 2(a) + 2(a) + 2(a) + 10(p)
    assert(getKPIpoints(A,UNI_A) == 84);
    // 82 - 10(p) for Uni C
    assert(getKPIpoints(A,UNI_C) == 72);

    disposeGame(A);
}


// return the number of students of the specified discipline type 
// the specified player currently has
void testGetStudents(void) {
    puts("Testing function getStudents()");
    int setResource[] = DEFAULT_DISCIPLINES;
    int setDice[] = DEFAULT_DICE;

    Game A = newGame(setResource, setDice);

    // Testing initial state
    assert(getStudents(A,UNI_A,STUDENT_THD) == 0);
    assert(getStudents(A,UNI_A,STUDENT_BPS) == 3);
    assert(getStudents(A,UNI_A,STUDENT_BQN) == 3);
    assert(getStudents(A,UNI_A,STUDENT_MJ) == 1);
    assert(getStudents(A,UNI_A,STUDENT_MTV) == 1);
    assert(getStudents(A,UNI_A,STUDENT_MMONEY) == 1);

    assert(getStudents(A,UNI_B,STUDENT_THD) == 0);
    assert(getStudents(A,UNI_B,STUDENT_BPS) == 3);
    assert(getStudents(A,UNI_B,STUDENT_BQN) == 3);
    assert(getStudents(A,UNI_B,STUDENT_MJ) == 1);
    assert(getStudents(A,UNI_B,STUDENT_MTV) == 1);
    assert(getStudents(A,UNI_B,STUDENT_MMONEY) == 1);

    assert(getStudents(A,UNI_C,STUDENT_THD) == 0);
    assert(getStudents(A,UNI_C,STUDENT_BPS) == 3);
    assert(getStudents(A,UNI_C,STUDENT_BQN) == 3);
    assert(getStudents(A,UNI_C,STUDENT_MJ) == 1);
    assert(getStudents(A,UNI_C,STUDENT_MTV) == 1);
    assert(getStudents(A,UNI_C,STUDENT_MMONEY) == 1);

    int count = 0;
    while (count < 100){
     assert(getStudents(A,UNI_A,STUDENT_MTV) == count + 1);
     throwDice(A,11);
     count++;
    }

    count = 0;
    while (count < 100){
      assert(getStudents(A,UNI_C,STUDENT_MTV) == count + 1);
      assert(getStudents(A,UNI_C,STUDENT_MJ) == count + 1);
      throwDice(A,8);
      count++;
    }

    count = 0;
    while (count < 100){
      assert(getStudents(A,UNI_B,STUDENT_BQN) == count + 3);
      throwDice(A,9);
      count++;
    }

    count = 0;
    while (count < 100){
      assert(getStudents(A,UNI_B,STUDENT_BPS) == count + 3);
      throwDice(A,5);
      count++;
    }

    count = 0;
    while (count < 100){
      assert(getStudents(A,UNI_A,STUDENT_MJ) == count + 1);
      throwDice(A,6);
      count++;
    }

}


// return the number of normal Campuses the specified player currently has
void testGetCampuses(void) {
    puts("Testing function getCampuses()");
    int setResource[] = DEFAULT_DISCIPLINES;
    int setDice[] = DEFAULT_DICE;

    Game A = newGame(setResource, setDice);

    // Testing initial state
    assert(getCampuses(A,UNI_A) == 2);
    assert(getCampuses(A,UNI_B) == 2);
    assert(getCampuses(A,UNI_C) == 2);

    runGame(A);

    // A Uni turn
    buildARC(A, "L");
    buildARC(A, "LR");
    buildARC(A, "LRR");
    buildARC(A, "LRRL");

    buildCampus(A, "LR");
    assert(getCampuses(A,UNI_A) == 3);
    buildCampus(A, "LRRL");
    assert(getCampuses(A,UNI_A) == 4);

    buildGO8(A, "LR");
    assert(getCampuses(A,UNI_A) == 3);
    buildGO8(A, "LRRL");
    assert(getCampuses(A,UNI_A) == 2);

    endTurn(A);

    // B Uni turn
    buildARC(A, "RRLRL");
    buildARC(A, "RRLR");
    buildARC(A, "RRLL");
    buildARC(A, "RRLLL");

    buildCampus(A, "RRL");
    assert(getCampuses(A,UNI_B) == 3);
    buildCampus(A, "RRLLL");
    assert(getCampuses(A,UNI_B) == 4);

    buildGO8(A, "RRL");
    assert(getCampuses(A,UNI_B) == 3);
    buildGO8(A, "RRLLL");
    assert(getCampuses(A,UNI_B) == 2);

    endTurn(A);

    // C Uni turn
    buildARC(A, "LRLRLR");
    buildARC(A, "LRLRLRR");
    buildARC(A, "LRLRLRRR");
    buildARC(A, "LRLRLRRRL");

    buildCampus(A, "LRLRLRR");
    assert(getCampuses(A,UNI_C) == 3);
    buildCampus(A, "LRLRLRRRL");
    assert(getCampuses(A,UNI_C) == 4);

    buildGO8(A, "LRLRLRR");
    assert(getCampuses(A,UNI_C) == 3);
    buildGO8(A, "LRLRLRRRL");
    assert(getCampuses(A,UNI_C) == 2);


    disposeGame(A);

    Game B = newGame(setResource, setDice);

    runGame(B);

    buildGO8(B,"LB");
    assert(getCampuses(B,UNI_A) == 1);
    buildGO8(B,"LRRLRLRLRLR");
    assert(getCampuses(B,UNI_A) == 0);

    disposeGame(B);
}


// test the makeAction() function
void testMakeAction(void) {
    puts("Testing function makeAction()...");

    int disciplines[] = RESOURCE_DISCIPLINES;
    int dice[] = RESOURCE_DICE;
    Game g = newGame (disciplines, dice);
    assert (g != NULL);
    assert (getTurnNumber(g) == -1);
    throwDice(g, 2);
    assert (getTurnNumber(g) == 0);

    // TEST 1: testing that the retrain action works
    genResources(g, 30, 2);
    action bqnToMJ = {.actionCode=RETRAIN_STUDENTS, 
            .disciplineFrom=STUDENT_BQN, .disciplineTo=STUDENT_MJ};
    makeAction(g, bqnToMJ);
    checkStudents(g, 94, 91, 2, 1, 1, 0);
    action bpsToMTV = {.actionCode=RETRAIN_STUDENTS, 
            .disciplineFrom=STUDENT_BPS, .disciplineTo=STUDENT_MTV};
    makeAction(g, bpsToMTV);
    // should now have 87 BQN, 87 BPS, 1 MJ, 1 MTV, nothing else
    checkStudents(g, 91, 91, 2, 2, 1, 0);

    // TEST 2: testing that the ip obtaining action works
    retrain(g, STUDENT_BQN, STUDENT_MJ, 10);
    retrain(g, STUDENT_BPS, STUDENT_MTV, 10);
    retrain(g, STUDENT_BPS, STUDENT_MMONEY, 10);
    // should now have 57 BQN, 57 BPS, 11 MJ, 11 MTV, 10 MMONEY 
    checkIPs(g, 0, 0, 0);
    action obtainIP = {.actionCode=OBTAIN_IP_PATENT};
    makeAction(g, obtainIP);
    checkIPs(g, 1, 0, 0);
    makeAction(g, obtainIP);
    checkIPs(g, 2, 0, 0);

    // TEST 3: testing that the publication obtaining works
    checkPubs(g, 0, 0, 0);
    action obtainPub = {.actionCode=OBTAIN_PUBLICATION};
    makeAction(g, obtainPub);
    checkPubs(g, 1, 0, 0);
    makeAction(g, obtainPub);
    checkPubs(g, 2, 0, 0);

    // TEST 4: testing that we can obtain ARCs
    action buildNewARC = {.actionCode=OBTAIN_ARC, .destination="R"};
    makeAction(g, buildNewARC);
    assert(getARCs (g, UNI_A) == 1);
    assert(getMostARCs (g) == UNI_A);
    assert(getARC (g, "R") == ARC_A);
    assert(getARC (g, "RL") == VACANT_ARC);
    assert(getARC (g, "RR") == VACANT_ARC);
    assert(getARC (g, "L") == VACANT_ARC);
    action buildSecondARC = {.actionCode=OBTAIN_ARC, .destination="RR"};
    makeAction(g, buildSecondARC);
    assert(getARCs (g, UNI_A) == 2);
    assert(getMostARCs (g) == UNI_A);
    assert(getARC (g, "R") == ARC_A);
    assert(getARC (g, "RL") == VACANT_ARC);
    assert(getARC (g, "RR") == UNI_A);
    assert(getARC (g, "L") == VACANT_ARC);

    // TEST 5: testing that we can build campuses
    Game g2 = newGame (disciplines, dice);
    assert (g2 != NULL);
    throwDice(g2, 2);
    assert(getTurnNumber (g2) == 0);
    assert(getWhoseTurn (g2) == UNI_A);
    // start the game with 2 campuses each
    checkCampuses(g2, 2, 2, 2);
    // get resources
    genResources(g2, 5, 2);
    // now have 16 BQN and 16 BPS. Use them to get ARCS
    buildARC(g2, "R");
    buildARC(g2, "RL");
    // retrain some students to build a campus
    retrain(g2, STUDENT_BQN, STUDENT_MJ, 1);
    retrain(g2, STUDENT_BPS, STUDENT_MTV, 1);
    action frstCampus = {.actionCode=BUILD_CAMPUS, .destination="RL"};
    makeAction(g2, frstCampus);
    // we should now have exactly 3 campuses, the others 2
    checkCampuses(g2, 3, 2, 2);
    // get more resources
    genResources(g2, 10, 2);
    buildARC(g2, "RLR");
    buildARC(g2, "RLRL");
    // retrain some students to build a campus
    retrain(g2, STUDENT_BQN, STUDENT_MJ, 1);
    retrain(g2, STUDENT_BPS, STUDENT_MTV, 1);
    action scndCampus = {.actionCode=BUILD_CAMPUS, .destination="RL"};
    makeAction(g2, scndCampus);
    // we should now have exactly 4 campuses, the others 2
    checkCampuses(g2, 4, 2, 2);

    // TEST 6: testing that we can build GO8s
    Game g3 = newGame (disciplines, dice);
    assert (g3 != NULL);
    throwDice(g3, 2);
    assert(getTurnNumber (g3) == 0);
    assert(getWhoseTurn (g3) == UNI_A);
    checkGO8s(g3, 0, 0, 0);
    // get resources
    genResources(g3, 5, 2);
    // now have 16 BQN and 16 BPS. Use them to get ARCS
    buildARC(g3, "R");
    buildARC(g3, "RL");
    // retrain some students to build a campus
    retrain(g3, STUDENT_BQN, STUDENT_MJ, 1);
    retrain(g3, STUDENT_BPS, STUDENT_MTV, 1);
    buildCampus(g3, "RL");
    // retrain some students to upgrade to GO8
    retrain(g3, STUDENT_BQN, STUDENT_MJ, 2);
    retrain(g3, STUDENT_BPS, STUDENT_MMONEY, 3);
    action firstGO8Action = {.actionCode=BUILD_GO8, .destination="RL"};
    makeAction(g3, firstGO8Action);
    // we should now have exactly one GO8 campus
    checkGO8s(g3, 1, 0, 0);
    // get more resources
    genResources(g3, 10, 2);
    buildARC(g3, "RLR");
    buildARC(g3, "RLRL");
    // retrain some students to build a campus
    retrain(g3, STUDENT_BQN, STUDENT_MJ, 1);
    retrain(g3, STUDENT_BPS, STUDENT_MTV, 1);
    buildCampus(g3, "RLRL");
    // retrain some students to upgrade to GO8
    retrain(g3, STUDENT_BQN, STUDENT_MJ, 2);
    retrain(g3, STUDENT_BPS, STUDENT_MMONEY, 3);
    action scndGO8Action = {.actionCode=BUILD_GO8, .destination="RLRL"};
    makeAction(g3, scndGO8Action);
    // we should now have exactly 2 GO8 campuses
    checkGO8s(g3, 2, 0, 0);

    disposeGame(g);
    disposeGame(g2);
    disposeGame(g3);
}


// test the getMostPublications() function
void testGetMostPublications(void) {
    puts("Testing function getMostPublications()...");

    int disciplines[] = RESOURCE_DISCIPLINES;
    int dice[] = RESOURCE_DICE;
    Game g = newGame (disciplines, dice);
    assert (g != NULL);
    throwDice(g, 2);

    // TEST SETUP: 20 MTV, 20 MJ, 20 MMONEY, 60 BPS
    genResources(g, 40, 2);
    retrain(g, STUDENT_BQN, STUDENT_MTV, 20);
    retrain(g, STUDENT_BQN, STUDENT_MJ, 20);
    retrain(g, STUDENT_BPS, STUDENT_MMONEY, 20);

    // TEST 1: at the start, no-one has the most publications
    assert(getMostPublications(g) == NO_ONE);

    // TEST 2: if UNI_A gets a publication on it's first turn, it will
    // have the most
    getPub(g, 1);
    assert(getMostPublications(g) == UNI_A);

    // TEST 3: if UNI_B then reaches the same number of publications
    // as the UNI_A, UNI_A still has the "most publications" bonus
    throwDice (g, 2);
    getPub(g, 1);
    assert(getMostPublications(g) != UNI_B);
    assert(getMostPublications(g) == UNI_A);

    // TEST 4: if UNI_C then gets two publications, it will have the
    // most publications
    throwDice (g, 2);
    getPub(g, 2);
    assert(getMostPublications(g) != UNI_A);
    assert(getMostPublications(g) == UNI_C);

    disposeGame(g);
}


// test the isLegalAction() function
void testIsLegalAction(void) {
    puts("Testing function isLegalAction()...");

    int disciplines[] = RESOURCE_DISCIPLINES;
    int dice[] = RESOURCE_DICE;
    Game g = newGame (disciplines, dice);
    assert (g != NULL);

    // TEST 1: it is illegal to make any action during terra nullis
    action a = {.actionCode = 0};
    action b = {.actionCode = 1};
    action c = {.actionCode = 2};
    action d = {.actionCode = 3};
    action e = {.actionCode = 4};
    action f = {.actionCode = 5};
    action gAction = {.actionCode = 6};
    action h = {.actionCode = 7, .disciplineFrom = STUDENT_BPS};
    assert(isLegalAction(g, a) == FALSE);
    assert(isLegalAction(g, b) == FALSE);
    assert(isLegalAction(g, c) == FALSE);
    assert(isLegalAction(g, d) == FALSE);
    assert(isLegalAction(g, e) == FALSE);
    assert(isLegalAction(g, f) == FALSE);
    assert(isLegalAction(g, gAction) == FALSE);
    assert(isLegalAction(g, h) == FALSE);

    // TEST 3: if we don't have enough students, the actions are illegal
    // we begin the game with 3 BPS, 3 BQN, 1 MTV, 1 MJ, 1 M$
    throwDice(g, 2);
    action q = {.actionCode = OBTAIN_ARC, .destination="R"};
    assert(isLegalAction(g, q) == TRUE);
    makeAction(g, q);
    action r = {.actionCode = OBTAIN_ARC, .destination="RR"};
    assert(isLegalAction(g, r) == TRUE);
    makeAction(g, r);
    action s = {.actionCode = BUILD_CAMPUS, .destination="RR"};
    assert(isLegalAction(g, s) == TRUE);
    makeAction(g, s);
    assert(getCampus(g, "RR") == CAMPUS_A);
    action t = {.actionCode = OBTAIN_ARC, .destination="RRL"};
    assert(isLegalAction(g, t) == TRUE);
    makeAction(g, t);
    // now have no students except MMONEY
    // can't retrain students since we have no students
    action t_retrain_a = {.actionCode=RETRAIN_STUDENTS, 
            .disciplineFrom=STUDENT_BPS, .disciplineTo=STUDENT_BQN};
    assert(isLegalAction(g, t_retrain_a) == FALSE);
    action t_retrain_b = {.actionCode=RETRAIN_STUDENTS, 
            .disciplineFrom=STUDENT_BQN, .disciplineTo=STUDENT_BPS};
    assert(isLegalAction(g, t_retrain_b) == FALSE);
    // can't get a spinoff since we have no students
    action t_start_spinoff = {.actionCode=START_SPINOFF};
    assert(isLegalAction(g, t_start_spinoff) == FALSE);
     
    // TEST 4: if the paths are invalid then the actions are illegal
    action v = {.actionCode = OBTAIN_ARC, .destination="RRR"};
    assert(isLegalAction(g, v) == FALSE);
    action w = {.actionCode = OBTAIN_ARC, .destination="RRRLLLR"};
    assert(isLegalAction(g, w) == FALSE);
    throwDice(g, 2);
    action buildARCOffBoard = {.actionCode = OBTAIN_ARC, .destination=
        "LRLRLRRLRLL"};
    isLegalAction(g, buildARCOffBoard);
    action buildARCOffBoard2 = {.actionCode = OBTAIN_ARC, .destination=
        "LRLRLRRLRL"};
    (isLegalAction(g, buildARCOffBoard2));
    throwDice(g, 2);
    throwDice(g, 2);
    
    // TEST 5: can't build ARCs/campuses on top of each other
    genResources(g, 5, 2);
    retrain(g, STUDENT_BQN, STUDENT_MJ, 1);
    retrain(g, STUDENT_BQN, STUDENT_MTV, 1);
    action x = {.actionCode = OBTAIN_ARC, .destination="RRL"};
    // there is already an ARC at RRL so the action is illegal
    assert(isLegalAction(g, x) == FALSE);
    action y = {.actionCode = BUILD_CAMPUS, .destination="RR"};
    // there is already a campus there so the action is illegal
    assert(isLegalAction(g, y) == FALSE);

    // TEST 6: can't build GO8 without having campus already there
    action z = {.actionCode = BUILD_GO8, .destination="RRL"};
    assert(isLegalAction(g, z) == FALSE);

    // TEST 7: can't build campus without having ARCS already there
    action aa = {.actionCode = BUILD_CAMPUS, .destination="LRRLRLR"};
    assert(isLegalAction(g, aa) == FALSE);

    // TEST 8: can't build campus neighbouring another campus 
    genResources(g, 50, 2);
    retrain(g, STUDENT_BQN, STUDENT_MJ, 20);
    retrain(g, STUDENT_BPS, STUDENT_MTV, 20);
    buildARC(g, "L");
    action ab = {.actionCode = BUILD_CAMPUS, .destination="L"};
    assert(isLegalAction(g, ab) == FALSE);
    buildARC(g, "LR");
    buildARC(g, "LRL");
    buildARC(g, "LRLR");
    action ac = {.actionCode = BUILD_CAMPUS, .destination="LRLR"};
    // this is next to the enemy campus
    assert(isLegalAction(g, ac) == FALSE);

    // TEST 9: it's illegal to outright get patents/ip
    action ad = {.actionCode = OBTAIN_PUBLICATION};
    assert(isLegalAction(g, ad) == FALSE);
    action ae = {.actionCode = OBTAIN_IP_PATENT};
    assert(isLegalAction(g, ae) == FALSE);

    // TEST 10: it's legal to pass
    action af = {.actionCode = PASS};
    assert(isLegalAction(g, af) == TRUE);
    disposeGame(g);
}


// test the getGO8s function
void testGetGO8s(void) {
    puts("Testing function getGO8s()...");

    int disciplines[] = RESOURCE_DISCIPLINES;
    int dice[] = RESOURCE_DICE;
    Game g = newGame (disciplines, dice);
    assert (g != NULL);
    throwDice(g, 2);

    // TEST 1: the game has just started so there are no GO8s
    assert(getTurnNumber (g) == 0);
    assert(getWhoseTurn (g) == UNI_A);
    checkGO8s(g, 0, 0, 0);

    // TEST 2: if the current player builds a campus, then upgrades it
    // to a GO8, getGO8s() should return 1 for them.
    // get resources
    genResources(g, 5, 2);
    // now have 16 BQN and 16 BPS. Use them to get ARCS
    buildARC(g, "R");
    buildARC(g, "RL");
    // retrain some students to build a campus
    retrain(g, STUDENT_BQN, STUDENT_MJ, 1);
    retrain(g, STUDENT_BPS, STUDENT_MTV, 1);
    buildCampus(g, "RL");
    // retrain some students to upgrade to GO8
    retrain(g, STUDENT_BQN, STUDENT_MJ, 2);
    retrain(g, STUDENT_BPS, STUDENT_MMONEY, 3);
    buildGO8(g, "RL");
    // we should now have exactly one GO8 campus
    checkGO8s(g, 1, 0, 0);

    // TEST 3: if the player builds another campus, then upgrades it
    // to a GO8, they should now have 2 GO8s
    // get resources
    genResources(g, 10, 2);
    buildARC(g, "RLR");
    buildARC(g, "RLRL");
    // retrain some students to build a campus
    retrain(g, STUDENT_BQN, STUDENT_MJ, 1);
    retrain(g, STUDENT_BPS, STUDENT_MTV, 1);
    buildCampus(g, "RLRL");
    // retrain some students to upgrade to GO8
    retrain(g, STUDENT_BQN, STUDENT_MJ, 2);
    retrain(g, STUDENT_BPS, STUDENT_MMONEY, 3);
    buildGO8(g, "RLRL");
    // we should now have exactly 2 GO8 campuses
    checkGO8s(g, 2, 0, 0);
    disposeGame(g);
}


// test the getPublications() function
void testGetPublications(void) {
    puts("Testing function getPublications()...");

    int disciplines[] = DEFAULT_DISCIPLINES;
    int dice[] = DEFAULT_DICE;
    Game g = newGame (disciplines, dice);
    assert (g != NULL);
    throwDice(g, 2);

    // TEST 1: the game has just started so there are no publications
    assert(getPublications(g, UNI_A) == 0);
    assert(getPublications(g, UNI_B) == 0);
    assert(getPublications(g, UNI_C) == 0);

    // TEST 2: add some publications to the current player and check
    // that they are counted
    getPub(g, 1);
    assert(getPublications(g, getWhoseTurn(g)) == 1);
    getPub(g, 1);
    assert(getPublications(g, getWhoseTurn(g)) == 2);
    getPub(g, 1);
    assert(getPublications(g, getWhoseTurn(g)) == 3);

    // TEST 3: change turn, add some publications to the next player
    // and check they are counted
    throwDice (g, 2);
    getPub(g, 1);
    assert(getPublications(g, getWhoseTurn(g)) == 1);
    getPub(g, 1);
    assert(getPublications(g, getWhoseTurn(g)) == 2);
    getPub(g, 1);
    assert(getPublications(g, getWhoseTurn(g)) == 3);

    // TEST 4: change turn, add some publications to the last player
    // and check they are counted
    throwDice (g, 2);
    getPub(g, 1);
    assert(getPublications(g, getWhoseTurn(g)) == 1);
    getPub(g, 1);
    assert(getPublications(g, getWhoseTurn(g)) == 2);
    getPub(g, 1);
    assert(getPublications(g, getWhoseTurn(g)) == 3);

    // TEST 5: change turn back to the first player and check the 
    //publications are still there
    throwDice (g, 2);
    getPub(g, 1);
    assert(getPublications(g, getWhoseTurn(g)) == 4);
    getPub(g, 1);
    assert(getPublications(g, getWhoseTurn(g)) == 5);
    getPub(g, 1);
    assert(getPublications(g, getWhoseTurn(g)) == 6);
    disposeGame(g);
}


// test the newGame function
void testNewGame(void) {
    puts("Testing function newGame()...");

    int disciplines[NUM_REGIONS] = DEFAULT_DISCIPLINES;
    int dice[NUM_REGIONS] = DEFAULT_DICE;

    Game g = newGame(disciplines, dice);

    assert(g != NULL);
    assert(getWhoseTurn(g) == NO_ONE);
    assert(getWhoseTurn(g) == NO_ONE);
    assert(getMostARCs(g) == NO_ONE);
    assert(getMostPublications(g) == NO_ONE);
    assert(getTurnNumber(g) == INITIAL_TURN);
    assert(getWhoseTurn(g) == NO_ONE);

    // there will be no ARCs assigned
    // at the start of a new game
    assert(getARC (g, "R") == VACANT_ARC);
    assert(getARC (g, "RL") == VACANT_ARC);
    assert(getARC (g, "RR") == VACANT_ARC);
    assert(getARC (g, "L") == VACANT_ARC);
    assert(getARC (g, "LR") == VACANT_ARC);
    assert(getARC (g, "LRL") == VACANT_ARC);
    assert(getARC (g, "LRR") == VACANT_ARC);
    assert(getARC (g, "LRLR") == VACANT_ARC);
    assert(getARC (g, "LRRL") == VACANT_ARC);
    assert(getARC (g, "RLRR") == VACANT_ARC);
    
    int i = UNI_A;
    int test[NUM_DISCIPLINES] = INITIAL_STUDENTS_NUM;
    while (i < NUM_UNIS) {
        assert(getKPIpoints(g,i) == INITIAL_KPI);
        assert(getARCs(g,i) == INITIAL_ARCS);
        assert(getGO8s(g,i) == INITIAL_GO8);
        assert(getCampuses(g,i) == NUM_INITIAL_CAMPUSES);
        assert(getIPs(g,i) == INITIAL_IP);
        assert(getPublications(g,i) == INITIAL_PUBLICATIONS);
        
        int j = 0;
        // tests that each uni will have the
        // requiSTUDENT_BPS starting resources (disciplines)
        while(j < NUM_DISCIPLINES){
            assert(getStudents(g,i,j) == test[j]);
            j++;
        }
        
        i++;
    }

    disposeGame(g);
}


// test the testGetDiscipline function
void testGetDiscipline(void) {
    puts("Testing function getDiscipline()...");

    int disciplines[NUM_REGIONS];
    int dice[NUM_REGIONS] = DEFAULT_DICE;

    int i = 0;
    while (i < NUM_REGIONS) {
        disciplines[i] = i % NUM_DISCIPLINES;
        i++;
    }

    Game game = newGame(disciplines, dice);

    i = 0;
    while (i < NUM_REGIONS) {
        assert(getDiscipline(game, i) == disciplines[i]);
        i++;
    }
    
    disposeGame(game);
}


// test the testGetDiceValue function
void testGetDiceValue(void) {
    puts("Testing function getDiceValue()...");

    int disciplines[NUM_REGIONS] = DEFAULT_DISCIPLINES;
    int dice[NUM_REGIONS];

    int i = 0;
    while (i < NUM_REGIONS) {
        dice[i] = ((i % MAX_DICE_VAL) + MIN_DICE_VAL) * NUM_DICE;
        i++;
    }

    Game g = newGame(disciplines, dice);

    i = 0;
    while (i < NUM_REGIONS) {
        assert(getDiceValue(g, i) == dice[i]);
        i++;
    }

    disposeGame(g);
}


// test the getTurnNumber function
void testGetTurnNumber(void) {
    puts("Testing function getTurnNumber()...");

    int disciplines[NUM_REGIONS] = DEFAULT_DISCIPLINES;
    int dice[NUM_REGIONS] = DEFAULT_DICE;

    Game g = newGame(disciplines, dice);

    int i = INITIAL_TURN;
    while (i < NUM_TURNS_TO_TEST) {
        assert(getTurnNumber(g) == i);
        throwDice(g, 2);
        i++;
    }
    disposeGame(g);
}


// test the getARC function
void testGetARC(void) {
    puts("Testing function getARCs()...");

    int disciplines[NUM_REGIONS] = RESOURCE_DISCIPLINES;
    int dice[NUM_REGIONS] = RESOURCE_DICE;

    Game g = newGame(disciplines, dice);
    genResources(g, 30, 2);
    throwDice(g, 2);

    assert(getMostARCs (g) == NO_ONE);
    action buildNewARC = {.actionCode=OBTAIN_ARC, .destination="L"};
    
    makeAction(g, buildNewARC);
    assert(getARCs (g, UNI_A) == 1);
    assert(getMostARCs (g) == UNI_A);
    assert(getARC (g, "R") == VACANT_ARC);
    assert(getARC (g, "RL") == VACANT_ARC);
    assert(getARC (g, "RR") == VACANT_ARC);
    assert(getARC (g, "L") == ARC_A);
    assert(getARC (g, "LR") == VACANT_ARC);
    assert(getARC (g, "LRL") == VACANT_ARC);
    assert(getARC (g, "LRR") == VACANT_ARC);
    assert(getARC (g, "LRLR") == VACANT_ARC);
    assert(getARC (g, "LRRL") == VACANT_ARC);
    assert(getARC (g, "RLRR") == VACANT_ARC);
    
    action buildSecondARC = {.actionCode=OBTAIN_ARC, .destination="RL"};
    
    makeAction(g, buildSecondARC);
    assert(getARCs (g, UNI_A) == 2);
    assert(getMostARCs (g) == UNI_A);
    assert(getARC (g, "R") == VACANT_ARC);
    assert(getARC (g, "RL") == ARC_A);
    assert(getARC (g, "RR") == VACANT_ARC);
    assert(getARC (g, "L") == ARC_A);
    assert(getARC (g, "LR") == VACANT_ARC);
    assert(getARC (g, "LRL") == VACANT_ARC);
    assert(getARC (g, "LRR") == VACANT_ARC);
    assert(getARC (g, "LRLR") == VACANT_ARC);
    assert(getARC (g, "LRRL") == VACANT_ARC);
    assert(getARC (g, "RLRR") == VACANT_ARC);

    action buildThirdARC = {.actionCode=OBTAIN_ARC, .destination="LRL"};
    
    makeAction(g, buildThirdARC);
    assert(getARCs (g, UNI_A) == 3);
    assert(getMostARCs (g) == UNI_A);
    assert(getARC (g, "R") == VACANT_ARC);
    assert(getARC (g, "RL") == ARC_A);
    assert(getARC (g, "RR") == VACANT_ARC);
    assert(getARC (g, "L") == ARC_A);
    assert(getARC (g, "LR") == VACANT_ARC);
    assert(getARC (g, "LRL") == ARC_A);
    assert(getARC (g, "LRR") == VACANT_ARC);
    assert(getARC (g, "LRLR") == VACANT_ARC);
    assert(getARC (g, "LRRL") == VACANT_ARC);
    assert(getARC (g, "RLRR") == VACANT_ARC);
    
    action buildFourthARC = {.actionCode=OBTAIN_ARC, .destination="RLRR"};
    
    makeAction(g, buildFourthARC);
    assert(getARCs (g, UNI_A) == 4);
    assert(getMostARCs (g) == UNI_A);
    assert(getARC (g, "R") == VACANT_ARC);
    assert(getARC (g, "RL") == ARC_A);
    assert(getARC (g, "RR") == VACANT_ARC);
    assert(getARC (g, "L") == ARC_A);
    assert(getARC (g, "LR") == VACANT_ARC);
    assert(getARC (g, "LRL") == ARC_A);
    assert(getARC (g, "LRR") == VACANT_ARC);
    assert(getARC (g, "LRLR") == VACANT_ARC);
    assert(getARC (g, "LRRL") == VACANT_ARC);
    assert(getARC (g, "RLRR") == ARC_A);
  
    disposeGame(g);
}


// test the getMostARCs function
void testGetMostARCS(void) {
    // ARC cost 1 STUDENT_BQN and 1 STUDENT_MMONEY
    puts("Testing function getMostArcs()...");
    //set up new game
    int disciplines[] = DEFAULT_DISCIPLINES;
    int dice[] = DEFAULT_DICE;
    Game g = newGame(disciplines, dice);

    //TEST 1: at the start, no-one has most Arcs
    assert(getMostARCs(g) == NO_ONE);

    runGame(g);
    buildARC(g, "L");

    //TEST 2: UNI_A has most arcs
    assert(getMostARCs(g) == UNI_A);

    endTurn(g);
    buildARC(g, "RRLRLL");

    //TEST 3: UNI_A has most arcs A = 1 B = 1
    assert(getMostARCs(g) == UNI_A);

    endTurn(g);
    buildARC(g, "RRLRLLRLRLL");

    //TEST 4: All have 1 Arc 
    assert(getMostARCs(g) == UNI_A);

    buildARC(g, "RRLRLLRLRLLR");
    
    //TEST 5: C has 2 Arcs
    assert(getMostARCs(g) == UNI_C);

    endTurn(g);
    buildARC(g, "LR");

    //TEST 6: A and C have 2 ARCs
    assert(getMostARCs(g) == UNI_C);
    disposeGame(g);
}


// test the getCampus function
void testGetCampus(void) {
    puts("Testing function getCampus()...");
    //set up new game
    int disciplines[] = DEFAULT_DISCIPLINES;
    int dice[] = DEFAULT_DICE;
    Game g = newGame(disciplines, dice);

    //TEST 1: Testing that each of the starting point has 1 campus
    assert(getCampus(g, "") == CAMPUS_A);
    assert(getCampus(g, "RRLRL") == CAMPUS_B);
    assert(getCampus(g, "LRLRL") == CAMPUS_C);
    assert(getCampus(g, "RRLRLLRLRL") == CAMPUS_C);
    assert(getCampus(g, "RRLRLLRLRLLRLRL") == CAMPUS_A);
    assert(getCampus(g, "RRLRLLRLRLLRLRLLRLRL") == CAMPUS_B);
    assert(getCampus(g, "RRLRLLRLRLLRLRLLRLRLLRLRL") == CAMPUS_C);

    //TEST 2: testing random points
    assert(getCampus(g, "L") == VACANT_VERTEX);
    assert(getCampus(g, "LR") == VACANT_VERTEX);
    assert(getCampus(g, "LRRLL") == VACANT_VERTEX);
    assert(getCampus(g, "LRRRRR") == CAMPUS_A);
    assert(getCampus(g, "LRRLLRRRLLR") == VACANT_VERTEX);

    runGame(g);

    buildARC(g, "L");
    buildARC(g, "LR");
    buildCampus(g, "LR");

    //TEST 3: testing new campus_A and surrounding not changed
    assert(getCampus(g, "LR") == CAMPUS_A);
    assert(getCampus(g, "L") == VACANT_VERTEX);
    assert(getCampus(g, "LRR") == VACANT_VERTEX);
    assert(getCampus(g, "LRL") == VACANT_VERTEX);

    throwDice(g, 11); //UNI_B's turn
    
    buildARC(g, "RRLRLL");
    buildCampus(g, "RRLRLL");

    //TEST 4: testing new campus_B
    assert(getCampus(g, "RRLRLL") == CAMPUS_B);

    buildGO8(g, "RRLRLL");
    //TEST 5: testing GO8_B
    assert(getCampus(g, "RRLRLL") == GO8_B);

    disposeGame(g);
}

// test the getARCs function
void testGetARCs(void) {
    puts("Testing function getARCs()...");
    //set up new game
    int disciplines[] = DEFAULT_DISCIPLINES;
    int dice[] = DEFAULT_DICE;
    Game g = newGame(disciplines, dice);

    //TEST 1: Testing that each player has no ARCs
    assert(getARCs(g, UNI_A) == NO_ONE);
    assert(getARCs(g, UNI_B) == NO_ONE);
    assert(getARCs(g, UNI_C) == NO_ONE);
    //assert(getARCs(g, NO_ONE) == NO_ONE); //maybe set NO_ONE to a random value
    
    runGame(g);

    buildARC(g, "L");
    buildARC(g, "LR");

    //TEST 2; Test new arcs for UNI_A
    assert(getARCs(g, UNI_A) == 2);
    assert(getARCs(g, UNI_B) == NO_ONE);
    assert(getARCs(g, UNI_C) == NO_ONE);

    endTurn(g); //UNI_B's turn

    buildARC(g, "RRLRLL");
    //TEST 3: Test new ARC for UNI_B
    assert(getARCs(g, UNI_A) == 2);
    assert(getARCs(g, UNI_B) == 1);
    assert(getARCs(g, UNI_C) == 0);

    endTurn(g);//UNI_C's turn

    buildARC(g, "RRLRLLRLRLL");
    //TEST 4: Test new ARC for UNI_C
    assert(getARCs(g, UNI_A) == 2);
    assert(getARCs(g, UNI_B) == 1);
    assert(getARCs(g, UNI_C) == 1);

    buildARC(g, "RRLRLLRLRLLR");
    //TEST 5: Test new ARC for UNI_C
    assert(getARCs(g, UNI_A) == 2);
    assert(getARCs(g, UNI_B) == 1);
    assert(getARCs(g, UNI_C) == 2);
    disposeGame(g);
}


// test the getIPs() function
void testGetIPs(void) {
    puts("Testing function getIPs()...");
    //set up new game
    int disciplines[] = DEFAULT_DISCIPLINES;
    int dice[] = DEFAULT_DICE;
    Game g = newGame(disciplines, dice);

    //TEST 1: Testing that each player has no IPs
    assert(getIPs(g, UNI_A) == 0);
    assert(getIPs(g, UNI_B) == 0);
    assert(getIPs(g, UNI_C) == 0);
    //assert(getIPs(g, NO_ONE) == 0); //maybe set NO_ONE to a random value

    runGame(g);

    action a = { .actionCode = 6 };
    makeAction(g, a);

    //TEST 2: Testing UNI_A has 1 IP
    assert(getIPs(g, UNI_A) == 1);
    assert(getIPs(g, UNI_B) == 0);
    assert(getIPs(g, UNI_C) == 0);

    makeAction(g, a);
    //TEST 3: Testing UNI_A has 2 IP
    assert(getIPs(g, UNI_A) == 2);
    assert(getIPs(g, UNI_B) == 0);
    assert(getIPs(g, UNI_C) == 0);

    endTurn(g);
    makeAction(g, a);

    //TEST 4: Testing UNI_B has 1 IP
    assert(getIPs(g, UNI_A) == 2);
    assert(getIPs(g, UNI_B) == 1);
    assert(getIPs(g, UNI_C) == 0);

    endTurn(g);
    makeAction(g, a);
    
    //TEST 5: Testing UNI_C has 1 IP
    assert(getIPs(g, UNI_A) == 2);
    assert(getIPs(g, UNI_B) == 1);
    assert(getIPs(g, UNI_C) == 1);
    disposeGame(g);
}


// test the getExchangeRate() function
void testGetExchangeRate(void) {
    puts("Testing function getExchangeRate()...");
    //set up new game
    int disciplines[] = DEFAULT_DISCIPLINES;
    int dice[] = DEFAULT_DICE;
    Game g = newGame(disciplines, dice);

    runGame(g);

    //TEST 1: exchanging STUDENT_BPS to other disciples
    assert(getExchangeRate(g, UNI_A, STUDENT_BPS, STUDENT_MTV) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_BPS, STUDENT_BQN) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_BPS, STUDENT_MJ) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_BPS, STUDENT_MMONEY) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_BPS, STUDENT_THD) == 3);

    //TEST 2: exchanging STUDENT_BQN to other disciples
    assert(getExchangeRate(g, UNI_A, STUDENT_BQN, STUDENT_BPS) == 3); 
    assert(getExchangeRate(g, UNI_A, STUDENT_BQN, STUDENT_BQN) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_BQN, STUDENT_MJ) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_BQN, STUDENT_THD) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_BQN, STUDENT_MMONEY) == 3);

    //TEST 3: exchanging STUDENT_MJ to other disciples
    assert(getExchangeRate(g, UNI_A, STUDENT_MJ, STUDENT_BPS) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_MJ, STUDENT_BQN) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_MJ, STUDENT_MTV) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_MJ, STUDENT_MMONEY) == 3); 
    assert(getExchangeRate(g, UNI_A, STUDENT_MJ, STUDENT_THD) == 3);

    //TEST 4: exchanging STUDENT_MTV to other disciples
    assert(getExchangeRate(g, UNI_A, STUDENT_MTV, STUDENT_BPS) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_MTV, STUDENT_BQN) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_MTV, STUDENT_MJ) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_MTV, STUDENT_MMONEY) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_MTV, STUDENT_THD) == 3);

    //TEST 5: exchanging STUDENT_MMONEY to other disciples
    assert(getExchangeRate(g, UNI_A, STUDENT_MMONEY, STUDENT_BPS) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_MMONEY, STUDENT_BQN) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_MMONEY, STUDENT_MJ) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_MMONEY, STUDENT_MTV) == 3);
    assert(getExchangeRate(g, UNI_A, STUDENT_MMONEY, STUDENT_THD) == 3);

    buildARC(g,"R");
    buildARC(g,"RR");
    buildCampus(g, "RR");

    //TEST 6: exchanging STUDENT_MTVN to other disciples on a training center
    assert(getExchangeRate(g, UNI_A, STUDENT_MTV, STUDENT_BPS) == 2);
    assert(getExchangeRate(g, UNI_A, STUDENT_MTV, STUDENT_BQN) == 2);
    assert(getExchangeRate(g, UNI_A, STUDENT_MTV, STUDENT_MJ) == 2);
    assert(getExchangeRate(g, UNI_A, STUDENT_MTV, STUDENT_MMONEY) == 2);
    assert(getExchangeRate(g, UNI_A, STUDENT_MTV, STUDENT_THD) == 2);

    endTurn(g);

    buildARC(g, "RRLRLLRLRLLRLRLLRLRLL");
    buildCampus(g, "RRLRLLRLRLLRLRLLRLRLL");

    //TEST 7: CAMPUS_B STUDENT_BQN for other 
    assert(getExchangeRate(g, UNI_B, STUDENT_BQN, STUDENT_BPS) == 2);
    assert(getExchangeRate(g, UNI_B, STUDENT_BQN, STUDENT_BQN) == 2);
    assert(getExchangeRate(g, UNI_B, STUDENT_BQN, STUDENT_MJ) == 2);
    assert(getExchangeRate(g, UNI_B, STUDENT_BQN, STUDENT_THD) == 2);
    assert(getExchangeRate(g, UNI_B, STUDENT_BQN, STUDENT_MMONEY) == 2);

    endTurn(g);

    buildARC(g, "RRLRLLRLRLL");
    buildCampus(g, "RRLRLLRLRLL");
    
    //TEST 8: CAMPUS_C STUDENT_BPS for other
    assert(getExchangeRate(g, UNI_C, STUDENT_BPS, STUDENT_MTV) == 2);
    assert(getExchangeRate(g, UNI_C, STUDENT_BPS, STUDENT_BQN) == 2);
    assert(getExchangeRate(g, UNI_C, STUDENT_BPS, STUDENT_MJ) == 2);
    assert(getExchangeRate(g, UNI_C, STUDENT_BPS, STUDENT_MMONEY) == 2);
    assert(getExchangeRate(g, UNI_C, STUDENT_BPS, STUDENT_THD) == 2);
    disposeGame(g);
}

/*
 * SOME FUNCTIONS WHICH SIMPLIFY THE TESTING BUT AREN'T PART OF THE 
 * TESTING SUITE NOR THE INTERFACE FOR THE ADT
 */


// Advances "count" number of rounds, generating students for the 
// players with campuses on hexagons with "diceNum" as the dice number.
// One round is three turns. A player has one turn per round.
void genResources(Game g, int count, int diceNum) {
    int i = 0;
    while (i < count*3) {
        throwDice(g, diceNum);
        i++;
    }
}


// builds an arc at the specified location
void buildARC(Game g, char *myPath) {
    action arcAction = {.actionCode=OBTAIN_ARC};
    strcpy(arcAction.destination, myPath);
    makeAction(g, arcAction);
}


// builds a campus at the specified location
void buildCampus(Game g, char *myPath) {
    action campusAction = {.actionCode=BUILD_CAMPUS};
    strcpy(campusAction.destination, myPath);
    makeAction(g, campusAction);
}


// upgrades a campus at the specified location to a GO8 
void buildGO8(Game g, char *myPath) {
    action go8Action = {.actionCode=BUILD_GO8};
    strcpy(go8Action.destination, myPath);
    makeAction(g, go8Action);
}


// grants publications to the player
void getPub(Game g, int count) {
    action givePub = {.actionCode = OBTAIN_PUBLICATION};
    int i = 0;
    while (i < count) {
        makeAction(g, givePub);
        i++;
    }
}


// retrains one lot of students from the given type to the new type
void retrain(Game g, int fromStudent, int toStudent, int amount) {
    action retrainAction = {.actionCode=RETRAIN_STUDENTS, 
            .disciplineFrom=fromStudent, .disciplineTo=toStudent};
    int i = 0;
    while (i < amount) {
        makeAction(g, retrainAction);
        i++;
    }
}


// check that the player has the resources they should
// in the order BPS, BQN, MJ, MTV, MMONEY, THD
void checkStudents(Game g, int bps, int bqn, int mj, int mtv,
        int mmoney, int thd) {
    assert (getStudents (g, getWhoseTurn(g), STUDENT_BPS) == bps);
    assert (getStudents (g, getWhoseTurn(g), STUDENT_BQN) == bqn);
    assert (getStudents (g, getWhoseTurn(g), STUDENT_MJ) == mj);
    assert (getStudents (g, getWhoseTurn(g), STUDENT_MTV) == mtv);
    assert (getStudents (g, getWhoseTurn(g), STUDENT_MMONEY) == mmoney);
    assert (getStudents (g, getWhoseTurn(g), STUDENT_THD) == thd);
}


// check that the unis have the number of IP patents they should 
void checkIPs(Game g, int uniAIp, int uniBIp, int uniCIp) {
    assert (getIPs (g, UNI_A) == uniAIp);
    assert (getIPs (g, UNI_B) == uniBIp);
    assert (getIPs (g, UNI_C) == uniCIp);
}


// check that the unis have the number of publications they should 
void checkPubs(Game g, int uniAPubs, int uniBPubs, int uniCPubs) {
    assert (getPublications (g, UNI_A) == uniAPubs);
    assert (getPublications (g, UNI_B) == uniBPubs);
    assert (getPublications (g, UNI_C) == uniCPubs);
}


// check that the unis have the number of campuses they should
void checkCampuses(Game g, int uniACmp, int uniBCmp, int uniCCmp) {
    assert (getCampuses (g, UNI_A) == uniACmp);
    assert (getCampuses (g, UNI_B) == uniBCmp);
    assert (getCampuses (g, UNI_C) == uniCCmp);
}

// check that the unis have the number of GO8s they should
void checkGO8s(Game g, int uniAGO8, int uniBGO8, int uniCGO8) {
    assert (getGO8s (g, UNI_A) == uniAGO8);
    assert (getGO8s (g, UNI_B) == uniBGO8);
    assert (getGO8s (g, UNI_C) == uniCGO8);
}


void runGame(Game g){
    //Let the game run to get enough resources

    throwDice(g, 11);
    assert(getTurnNumber(g) == 0);
    int turns = 0;
    int retrainStu;
    int toStud;
    assert(getWhoseTurn(g) == UNI_A);

    while (turns++ < 360){
        throwDice(g, 11); //uni++
    }//UNI_A gets 360 STUDENT_MTVNS total  361 STUDENT_MTVN
    assert(getWhoseTurn(g) == UNI_A);

    //after throw dice it's UNI_A's turn since 108 is divisable by 3 (3 players)
    
    int counter = 0;
    retrainStu = STUDENT_MTV;
    toStud = STUDENT_BPS;
    while (counter++ < 116){
        if (toStud>5){
            toStud = STUDENT_BPS;
        }
        if (toStud == STUDENT_MTV){
            toStud++;
        }
        retrain(g,retrainStu,toStud, 1);
        toStud++;
    }//exchanging students, left with 15 STUDENT_MTVN and 29 each (without initial values)

    turns = 0;
    while (turns++ < 360){
        throwDice(g, 5);
    }//UNI_B gets 360 STUDENT_BPS total 363 STUDENT_BPS
    throwDice(g, 5);
    assert(getWhoseTurn(g) == UNI_B);


    counter = 0;
    retrainStu = STUDENT_BPS;
    toStud = STUDENT_BQN;
    while (counter++ < 116){
        if (toStud>5){
            toStud = STUDENT_BQN;
        }
        retrain(g, retrainStu, toStud, 1);
        toStud++;
    }//exchanging students, left with 18 STUDENT_BPS and 29 EACH
    throwDice(g, 5);
    assert(getWhoseTurn(g) == UNI_C);

    turns = 0;
    while (turns++ < 360){
        throwDice(g, 8);
    }//UNI_C gets 360 STUDENT_MTVNS,360 STUDENT_MJOW

    counter = 0;
    retrainStu = STUDENT_MTV;
    toStud = STUDENT_BPS;
    while (counter++ < 116){
        if (toStud>5){
            toStud = STUDENT_BPS;
        }
        if (toStud == STUDENT_MTV){
            toStud++;
        }
        retrain(g, retrainStu, toStud, 1);
        toStud++;
    }//exchanging students, left with 15 STUDENT_MTVN and 29 each (without initial values) 

    throwDice(g, 11);//UNI_A's turn
}


//Ends turn and genrates a STUDENT_MTVn for CAMPUS_A
void endTurn(Game g){
    throwDice(g, 11);
}
