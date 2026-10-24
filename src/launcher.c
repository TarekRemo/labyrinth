#include <stdlib.h>
#include "launcher.h"
#include "generator.h"
#include "cfg_io.h"
#include "labyrinth.h"
#include "string.h"

#define MAX_MENU_MSG_LENGTH 150

Labyrinth* loaded_labyrinth;

char* error_msg;
char* success_msg; 

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
static MenuChoice show_menu(void);

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
static void create_labyrinth(void);

/**
 * Affiche un menu qui permet à l'utilisateur de choisir une difficulté.
 * 
 * @return la difficulté choisie
 */
static Difficulty read_labyrinth_difficulty(void);

/**
 * Permet de quitter le launcher avec un code de sortie.
 * 
 * @param exit_code le code de sortie. 
 */
static void quit(int exit_code);

void launch(){
    error_msg = malloc(MAX_MENU_MSG_LENGTH);
    success_msg = malloc(MAX_MENU_MSG_LENGTH);

    loaded_labyrinth = NULL;
    MenuChoice choice;

    while(1){
        choice = show_menu();
        error_msg = NULL;
        success_msg = NULL;

        switch(choice){
            case CREATE_LABYRINTH:
                create_labyrinth();
                break;
            
            case QUIT:
                quit(0);
        }
    }
}

MenuChoice show_menu(void){
    set_canonical_mode(0);

    //Choix par défaut = jouer si un labyrinthe est chargé, créer un labyrinthe sinon
    MenuChoice current_choice = loaded_labyrinth != NULL ? PLAY : CREATE_LABYRINTH;

    KeyboardKey key;
    int modifier = 0;
    while(1){
        set_background_color(MENU_BACKGROUND_COLOR);

        if(error_msg != NULL){
            print_text(
                error_msg,
                MENU_ERROR_TEXT_COLOR,
                0,
                MENU_BACKGROUND_COLOR,
                1
            );
        }

        if(success_msg != NULL){
            print_text(
                success_msg,
                MENU_SUCCESS_TEXT_COLOR,
                0,
                MENU_BACKGROUND_COLOR,
                1
            );
        }

        print_menu_choices(current_choice);

        //On attend une entrée reconnue
        while((key=read_entry()) == NONE || key == LEFT || key == RIGHT);

        if(key == UP || key == DOWN){
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
            set_background_color(MENU_BACKGROUND_COLOR);
            return current_choice;
        }
        else if(key == R){
            return RETURN;
        }
    }
}

static void print_menu_choices(MenuChoice current_choice){
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
    if(loaded_labyrinth != NULL){
        free(loaded_labyrinth);
        loaded_labyrinth = NULL;
    }

    set_background_color(MENU_BACKGROUND_COLOR);
    set_canonical_mode(1);

    char name[MAX_LABYRINTH_NAME_LENGTH];
    print_text(
        "Entrer le nom du labyrinthe à créer",
        MENU_CHOICE_COLOR,
        0,
        MENU_BACKGROUND_COLOR,
        1
    );
    
    char* name_end;
    while(
        fgets(name, sizeof(name), stdin) == NULL ||
        (name_end = strchr(name, '\n')) == NULL
    ){
        print_text(
            "Veulliez entrer un nom qui fait moins de 150 caractères",
            MENU_ERROR_TEXT_COLOR,
            0,
            MENU_BACKGROUND_COLOR,
            1
        );
        char c;
        while((c = getc(stdin)) != '\0' && c != '\n' && c != EOF);
    }
    *name_end = '\0';
    
    set_background_color(MENU_BACKGROUND_COLOR);

    int height, width;
    Difficulty difficulty;
    print_text(
        "Entrer la hauteur et la largeur du labyrinthe à créer",
        MENU_CHOICE_COLOR,
        0,
        MENU_BACKGROUND_COLOR,
        1
    );

    while(
        read_int(stdin, &height) == 0 ||
        read_int(stdin, &width) == 0
    ){
        set_background_color(MENU_BACKGROUND_COLOR);
        print_text(
            "Veillez entrer des entiers valides et impairs",
            MENU_ERROR_TEXT_COLOR,
            0,
            MENU_BACKGROUND_COLOR,
            1
        );
    }

    difficulty = read_labyrinth_difficulty();

    LabyrinthStatus generate_status = generate_labyrinth(&loaded_labyrinth, difficulty, height, width);
    switch(generate_status){
        case LABYRINTH_SUCCESS: 
            break;

        case LABYRINTH_CORRUPTED:
            error_msg = "La taille minimale du labyrinthe est de 3x3 et La hauteur et la largeur du labyrinthe doivent être impaires.";
            return;

        case LABYRINTH_MEMORY_ERROR:
            error_msg = "Impossible d'allouer la mémoire nécessaire pour la génération du labyrinthe.";
            return;

        default:
            error_msg = "Impossible de construire le labyrinthe.";
            return;
    }

    LabyrinthStatus save_status = save_labyrinth(name, loaded_labyrinth);
    switch(save_status){
        case LABYRINTH_SUCCESS:
            success_msg = "Labyrinthe crée, chargé et sauvegardé avec succès.";
            break;

        case LABYRINTH_ALREADY_EXISTS:
            error_msg = "Un labyrinthe avec ce nom existe déjà";
            free_labyrinth(loaded_labyrinth);
            loaded_labyrinth = NULL;
            break;

        case LABYRINTH_CORRUPTED:
            error_msg = "Les paramètres du labyrinth sont invalides";
            free_labyrinth(loaded_labyrinth);
            loaded_labyrinth = NULL;
            break;

        case LABYRINTH_INVALID_NAME:
            error_msg = "Le nom entré n'est pas valide";
            free_labyrinth(loaded_labyrinth);
            loaded_labyrinth = NULL;
            break;

        case LABYRINTH_PERMISSION_ERROR:
            error_msg = "Une erreur de permission empêche la sauvegarde du labyrinthe";
            free_labyrinth(loaded_labyrinth);
            loaded_labyrinth = NULL;
            break;
        
        default:
            error_msg = "Une erreur s'est produite.";
            free_labyrinth(loaded_labyrinth);
            loaded_labyrinth = NULL;
            break;
    }
}

static Difficulty read_labyrinth_difficulty(){
    set_canonical_mode(0);
    Difficulty difficulty = EASY;
    KeyboardKey key;
    while(1){
        set_background_color(MENU_BACKGROUND_COLOR);

        print_text(
            "Facile",
            MENU_CHOICE_COLOR,
            0,
            difficulty == EASY ? MENU_SELECTED_CHOICE_COLOR : MENU_BACKGROUND_COLOR,
            1
        );

        print_text(
            
            "Difficile",
            MENU_CHOICE_COLOR,
            0,
            difficulty == HARD ? MENU_SELECTED_CHOICE_COLOR : MENU_BACKGROUND_COLOR,
            1
        );

        //On attend une entrée reconnue
        while((key=read_entry()) == NONE || key == LEFT || key == RIGHT || key == R);

        if(key == UP || key == DOWN){//flèche du haut
            difficulty = difficulty == EASY ? HARD : EASY;
        }
        else if (key == ENTER){
            set_background_color(MENU_BACKGROUND_COLOR);
            return difficulty;
        }
    }
}

static void quit(int exit_code){
    set_canonical_mode(1);
    free_labyrinth(loaded_labyrinth);
    free(error_msg);
    free(success_msg);
    exit(exit_code);
}

