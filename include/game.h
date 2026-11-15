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
#define GAME_PENALTY_PER_MVMT 3
#define GAME_TRAP_PENALTY 75
#define GAME_TREASURE_BONUS 50

typedef enum GAMESTATUS{
    NOT_STARTED,
    NOT_FINISHED,
    WON
} GAMESTATUS;

/**
 * Lance une partie.
 * 
 * @param labyrinth le labyrinthe à lancer.
 * @param nb_mvmts le nombre de déplacements faits depuis le début de la prtie
 * @param current_score score actuel
 * 
 * @return l'état de la partie (NOT_STARTED, NOT_FINISHED, WON)
 */
GAMESTATUS launch_game(Labyrinth* labyrinth, long int* nb_mvmts, long int* current_score);

#endif