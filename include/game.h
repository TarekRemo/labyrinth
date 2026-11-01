#ifndef GAME_H
#define GAME_H "game.h"

#include <labyrinth.h>

#define GAME_BACKGROUND_COLOR BLACK
#define GAME_WALL_COLOR GRAY
#define GAME_PLAYER_COLOR GRAY
#define GAME_DOOR_COLOR GRAY
#define GAME_KEY_COLOR GRAY
#define GAME_TREASURE_COLOR GRAY
#define GAME_TRAP_COLOR GRAY

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
 * 
 * @return l'état de la partie (NOT_STARTED, NOT_FINISHED, WON, LOST)
 */
GAMESTATUS launch_game(Labyrinth* labyrinth);

#endif