#ifndef LAUNCHER_H
#define LAUNCHER_H "launcher.h"

#include "labyrinth.h"
#include "utils.h"

#define MENU_BACKGROUND_COLOR BLACK
#define MENU_CHOICE_COLOR PINK
#define MENU_DISABLED_CHOICE_COLOR GRAY
#define MENU_SELECTED_CHOICE_COLOR GREEN
#define MENU_ERROR_TEXT_COLOR RED
#define MENU_SUCCESS_TEXT_COLOR GREEN

/**
 * Enum représentant les différents choix possibles dans le menu du jeu.
 */
typedef enum MenuChoice {
    PLAY,
    CREATE_LABYRINTH,
    LOAD_LABYRINTH,
    QUIT,
    RETURN
} MenuChoice;

/**
 * Pointeur vers le labyrinthe actuellement chargé en mémoire.
 */
extern Labyrinth* loaded_labyrinth;

/**
 * Si la valeur de ce pointeur n'est pas null, 
 * alors il contient un message d'erreur à afficher dans le menu.
 */
extern char* error_msg;

/**
 * Si la valeur de ce pointeur n'est pas null, 
 * alors il contient un message de réussite à afficher dans le menu.
 */
extern char* success_msg; 

/**
 * Lance le jeu.
 * 
 * @return void
 */
void launch(void);

#endif