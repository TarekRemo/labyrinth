#ifndef GAME_H
#define GAME_H "game.h"

#include <labyrinth.h>

#define GAME_BACKGROUND_COLOR GRAY_GREEN
#define GAME_WALL_COLOR DARK_GREEN
#define GAME_PLAYER_COLOR BLUE
#define GAME_DOOR_COLOR BROWN
#define GAME_KEY_COLOR YELLOW
#define GAME_TREASURE_COLOR GREEN
#define GAME_TRAP_COLOR RED

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