#include <stdlib.h>
#include "launcher.h"
#include "generator.h"

Labyrinth* loaded_labyrinth;

/**
 * Affiche les différents choix possibles du menu sur la sortie standard.
 * 
 * Le choix selectionné par current_choice est surligné. 
 * Si aucun labyrinthe n'est chargé en mémoire (loaded_labyrinth = NULL), alors le choix de jouer est désactivé.
 * 
 * @param current_choice le choix sélectionné.
 * 
 * @return void
 */
static void print_menu_choices(MenuChoice current_choice);

/**
 * Demande à l'utilisateur d'entrer les informations du labyrinthe à créer
 * puis le génère, le charge en mémoire (dans loaded_labyrinth) et l'enregistre dans un fichier cfg.
 * 
 * @return void  
 */
static void create_labyrinth();

void launch(){
    loaded_labyrinth = NULL;
    int game_already_launched = 0;
    MenuChoice choice;

    while(1){
        choice = show_menu();

        switch(choice){
            case CREATE_LABYRINTH:
                create_labyrinth();
                break;
        }
    }
}

MenuChoice show_menu(void){
    set_canonical_mode(0);

    //Choix par défaut = créer un labyrinthe
    MenuChoice current_choice = loaded_labyrinth != NULL ? PLAY : CREATE_LABYRINTH;

    KeyboardKey key;
    int modifier = 0;
    while(1){
        print_choices(current_choice);

        //On attend une entrée reconnue
        while((key=read_entry()) == NONE || key == LEFT || key == RIGHT);

        if(key == UP || key == DOWN){//flèche du haut
            modifier = key == UP ? 1 : -1;

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
            continue;
        }
        else if (key == ENTER){
            return current_choice;
        }
        else if(key == ESCAPE){
            return RETURN;
        }
    }
}

static void print_menu_choices(MenuChoice current_choice){
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

static void create_labyrinth(){
    set_canonical_mode(1);
    set_background_color(MENU_BACKGROUND_COLOR);

    print_text(
        "Entrer le nom du labyrinthe à créer",
        MENU_CHOICE_COLOR,
        0,
        MENU_BACKGROUND_COLOR,
        1
    );

    char* name[MAX_LABYRINTH_NAME_LENGTH];
    while(1){
        scanf("%s", name);
        set_background_color(MENU_BACKGROUND_COLOR);

        if(!is_valid_labyrinth_name(name)){
                print_text(
                "Ce nom est invalide, veuillez réessayer.",
                ERROR_TEXT_COLOR,
                0,
                MENU_BACKGROUND_COLOR,
                1
            );
            continue;
        }
    }
}
