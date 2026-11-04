#ifndef GAME_H
#define GAME_H "game.h"

#include <labyrinth.h>

#define GAME_BACKGROUND_COLOR GRAY_GREEN
#define GAME_WALL_COLOR BROWN
#define GAME_PLAYER_COLOR BLUE
#define GAME_DOOR_COLOR BLACK
#define GAME_KEY_COLOR YELLOW
#define GAME_TREASURE_COLOR GREEN
#define GAME_TRAP_COLOR RED

#define GAME_MAX_SCORE 500
#define GAME_PENALTY_PER_SEC 2
#define GAME_TRAP_PENALTY 75
#define GAME_TREASURE_BONUS 50

typedef enum GAMESTATUS{
    NOT_STARTED,
    NOT_FINISHED,
    WON,
    LOST
} GAMESTATUS;

/**
 * Lance une partie.
 * 
 * @param labyrinth le labyrinthe à lancer.
 * @param elapsed_time le temps écoulé depuis le début de la prtie
 * @param current_score score actuel
 * 
 * @return l'état de la partie (NOT_STARTED, NOT_FINISHED, WON, LOST)
 */
GAMESTATUS launch_game(Labyrinth* labyrinth, long int* elapsed_time, long int* current_score);

#endif