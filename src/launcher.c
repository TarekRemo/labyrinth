#include <stdlib.h>
#include "launcher.h"

MenuChoice current_choice;
Labyrinth* loaded_labyrinth;

/**
 * Affiche les différents choix possibles du menu sur la sortie standard.
 * 
 * Le choix selectionné par current_choice est surligné. 
 * Si aucun labyrinthe n'est chargé en mémoire (loaded_labyrinth = NULL), alors le choix de jouer est désactivé.
 *  
 * @return void
 */
static void print_choices(void);

/**
 * Récupère un caractère depuis l'entrée standard et modifie current_choice en fonction de ce caractère.
 * 
 * @return void
 */
static void get_choice(void);

void launch_menu(void){
    //Choix par défaut = créer un labyrinthe
    current_choice = loaded_labyrinth != NULL ? PLAY : CREATE_LABYRINTH;

    set_canonical_mode(0);
    while(1){
        print_choices();
        get_choice();
    }
}

static void print_choices(void){
    set_background_color(MENU_BACKGROUND_COLOR);

    print_text(
        "Jouer",
        loaded_labyrinth != NULL ? MENU_CHOICE_COLOR : MENU_DISABLED_CHOICE_COLOR,
        0,
        current_choice == PLAY ? MENU_SELECTED_CHOICE_COLOR : MENU_BACKGROUND_COLOR,
        1
    );

    print_text(
        "Créer un labyrinthe",
        MENU_CHOICE_COLOR,
        0,
        current_choice == CREATE_LABYRINTH ? MENU_SELECTED_CHOICE_COLOR : MENU_BACKGROUND_COLOR,
        1
    );
    
    print_text(
        "Charger un labyrinth",
        MENU_CHOICE_COLOR,
        0,
        current_choice == LOAD_LABYRINTH ? MENU_SELECTED_CHOICE_COLOR : MENU_BACKGROUND_COLOR,
        1
    );    

    print_text(
        "Quitter",
        MENU_CHOICE_COLOR,
        0,
        current_choice == QUIT ? MENU_SELECTED_CHOICE_COLOR : MENU_BACKGROUND_COLOR,
        1
    );    
}

static void get_choice(){
    char c = getchar();

    //Si le caractère commenec par esac (c'est le cas pour les flèches du haut et du bas)
    if(c == 27){
        getchar(); //ignorer le deuxième caractère
        c = getchar(); //le 3ème permet de connaître la touche

        int modifier = 0;
        if(c == 'A'){//flèche du haut
            modifier++;
        } 
        else if(c == 'B'){//flèche du bas
            modifier--;
        }
        
        if(current_choice == PLAY && modifier == 1){
            current_choice = QUIT;
        }
        else if(current_choice == QUIT && modifier == -1){
            current_choice = PLAY;
        }
        else{
            current_choice += modifier;
        }

        //Impossible de jouer si aucun labyrinthe n'est chargé
        if(loaded_labyrinth == NULL && current_choice == PLAY){
            current_choice = CREATE_LABYRINTH;
        }
    }
}