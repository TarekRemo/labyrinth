#ifndef LAUNCHER_H
#define LAUNCHER_H "launcher.h"

#include "labyrinth.h"
#include "utils.h"

#define MENU_BACKGROUND_COLOR BLACK
#define MENU_CHOICE_COLOR PINK
#define MENU_DISABLED_CHOICE_COLOR GRAY
#define MENU_SELECTED_CHOICE_COLOR GREEN

/**
 * Enum représentant les différents choix possibles dans le menu du jeu.
 */
typedef enum MenuChoice {
    QUIT,
    LOAD_LABYRINTH,
    CREATE_LABYRINTH,
    PLAY
} MenuChoice;

/**
 * Variable contenant le choix actuellement sélectionné dans le menu.
 */
extern MenuChoice current_choice;

/**
 * Pointeur vers le labyrinthe actuellement chargé en mémoire.
 */
extern Labyrinth* loaded_labyrinth;

/**
 * Lance et affiche le menu du jeu.
 * 
 * @return void
 */
void launch_menu(void);

#endif