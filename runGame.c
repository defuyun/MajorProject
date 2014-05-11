/* runGame.c - interactive frontend for the game
 * By Timothy Chin, James Houlahan, Carl Liu and Matthew Siesco
 * 
 * Runs a simulation of the game, but takes in user input instead of
 * calling AI functions. Can be used for debugging.
 * 
 * Action codes will be determined by scanf's
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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


// ask the user for an action
action getAction(void);
void printResources(Game g, int player);


// ask each player for an action
int main (int argc, char *argv[]){
    // seed the random number generator
    srand(time(NULL));

    // get a new game
    int disciplines[] = DEFAULT_DISCIPLINES;
    int dice[] = DEFAULT_DICE;
    Game g = newGame(disciplines, dice);

    int hasWinner = FALSE;
    int winner = NO_ONE;
    while (hasWinner == FALSE) {
        throwDice(g, (rand()%6 + rand()%6 + 2));
        printf("Player %d's turn\n", getWhoseTurn(g));
        printResources(g, getWhoseTurn(g));

        // get the player's action
        action a = getAction();
        while (isLegalAction(g, a) == FALSE) {
            printf("Illegal action\n");
            a = getAction();
        }

        // if they wanted a spinoff, calculate the chance outcome
        if (a.actionCode == START_SPINOFF) {
            if (rand()%3 == 0) {
                a.actionCode = OBTAIN_IP_PATENT;
            } else {
                a.actionCode = OBTAIN_PUBLICATION;
            }
        }

        // make their action and check if they won
        makeAction(g, a);
        if (getKPIpoints(g, getWhoseTurn(g)) >= 150) {
            hasWinner = TRUE;
            winner = getWhoseTurn(g);
        }
    }

    printf("Player %d won\n", winner);

    return EXIT_SUCCESS;
}


// ask the user for an action
action getAction(void) {
    action a;

    printf("Enter an action to perform: ");
    scanf("%d", &(a.actionCode));

    // depending on the action, get more information
    if (a.actionCode == BUILD_CAMPUS || a.actionCode == OBTAIN_ARC
            || a.actionCode == BUILD_GO8) {
        printf("Enter a path: ");
        path p;
        scanf("%s", p);
        strcpy(a.destination, p);
    } else if (a.actionCode == RETRAIN_STUDENTS) {
        printf("Enter student type to retrain from: ");
        scanf("%d", &(a.disciplineFrom));
        printf("Enter student type to retrain to: ");
        scanf("%d", &(a.disciplineTo));
    }

    return a;
}


// print current resources tally
void printResources(Game g, int player) {
    printf("Player %d has resources:\n", getWhoseTurn(g));
    printf("\t%d THD\n\t%d BPS\n\t%d BQN\n\t%d MJ\n\t%d MTV\n\t%d "
            "MMONEY\n", getStudents(g, player, STUDENT_THD),
            getStudents(g, player, STUDENT_BPS),
            getStudents(g, player, STUDENT_BQN),
            getStudents(g, player, STUDENT_MJ),
            getStudents(g, player, STUDENT_MTV),
            getStudents(g, player, STUDENT_MMONEY));
}
