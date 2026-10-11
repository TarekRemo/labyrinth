#ifndef LAUNCHER_H
#define LAUNCHER_H "launcher.h"

#include "labyrinth.h"
#include "utils.h"

#define MENU_BACKGROUND_COLOR BLACK
#define MENU_CHOICE_COLOR PINK
#define MENU_DISABLED_CHOICE_COLOR GRAY
#define MENU_SELECTED_CHOICE_COLOR GREEN
#define ERROR_TEXT_COLOR RED

/**
 * Enum représentant les différents choix possibles dans le menu du jeu.
 */
typedef enum MenuChoice {
    QUIT,
    RETURN,
    LOAD_LABYRINTH,
    CREATE_LABYRINTH,
    PLAY
} MenuChoice;

/**
 * Pointeur vers le labyrinthe actuellement chargé en mémoire.
 */
extern Labyrinth* loaded_labyrinth;

/**
 * Lance le jeu.
 * 
 * @return void
 */
void launch(void);

/**
 * Affiche le menu du jeu et attend que l'utilisateur selectionne un choix du menu.
 * 
 * L'utilisateur navigue entre les différents choix en utilisant les flèches du clavier et 
 * valide un choix en appuyant sur entrée.
 * 
 * L'utilisateur peut aussi mettre fin à la fonction sans faire aucun choix en appuant sur echape.
 * 
 * @return void
 */
MenuChoice show_menu(void);

#endif