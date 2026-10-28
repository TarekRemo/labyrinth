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
 * L'utilisateur peut aussi mettre fin à la fonction sans faire aucun choix en appuant sur r.
 * 
 * @return void
 */
static MenuChoice show_menu(void);

/**
 * Affiche une liste de choix sur la sortie standard.
 * 
 * Chaque choix est représenté par son indice dans le tableau choices_list.
 * Le choix selectionné par current_choice est surligné. 
 * Les choix désactivés présents dans le tableau disabled_choices seront affichés avec une couleur différente indiquant cela.
 * 
 * @param choices_list la liste des choix à afficher
 * @param nb_choices le nombre de choix à afficher
 * @param current_choice le choix sélectionné
 * @param nb_disabled_choices le nombre de choix désactivés
 * @param disabled_choices la liste des choix désactivés
 * 
 * @return void
 */
static void print_choices(char** choices_list, int nb_choices, int current_choice, int nb_disabled_choices, int* disabled_choices);

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
 * Affiche la liste des labyrinthes enregistrés et permet d'en choisir un à charger en mémoire.
 * 
 * @return void
 */
static void choose_labyrinth(void);

/**
 * Affiche une liste de labyrinthes et attend que l'utilisateur en choisit un.
 * 
 * L'utilisateur navigue entre les différents choix en utilisant les flèches du clavier et 
 * valide un choix en appuyant sur entrée.
 * 
 * Si l'utilisateur appuie sur r, cette fonction retourne -1 pour indiquer la volonté de retourner en arrière.
 * 
 * @param labyrinths_list la liste de labyrinthes à afficher
 * @param nb_labyrinths le nombre de labyrinthes à afficher
 * 
 * @return le choix de l'utilisateur ou -1 si aucun choix n'est fait.
 */
static int show_labyrinths_list(char** labyrinths_list, int nb_labyrinths);

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

            case LOAD_LABYRINTH:
                choose_labyrinth();
                break;
            
            case QUIT:
                quit(0);

            default: 
                break;
        }
    }
}

MenuChoice show_menu(void){
    set_canonical_mode(0);

    int labyrinth_is_loaded = loaded_labyrinth != NULL ? 1 : 0;
    MenuChoice current_choice = labyrinth_is_loaded ? PLAY : CREATE_LABYRINTH;

    char* menu_choices[] = {
        "Jouer",
        "Créer un labyrinthe",
        "Charger un labyrinth",
        "Quitter"
    };

    int disabled_choices[] = {0};

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

        labyrinth_is_loaded = loaded_labyrinth != NULL ? 1 : 0;
        print_choices(menu_choices, 4, current_choice, labyrinth_is_loaded ? 0 : 1, labyrinth_is_loaded ? NULL : disabled_choices);

        //On attend une entrée reconnue
        while((key=read_entry()) == NONE || key == LEFT || key == RIGHT);

        if(key == UP || key == DOWN){
            modifier = key == UP ? -1 : 1;

            if(current_choice == PLAY && modifier == -1){
                current_choice = QUIT;
            }
            else if(current_choice == QUIT && modifier == 1){
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

    char* difficulty_choices[] = {
        "Facile",
        "Difficile"
    };
    
    Difficulty difficulty = EASY;
    KeyboardKey key;
    while(1){
        set_background_color(MENU_BACKGROUND_COLOR);

        print_choices(difficulty_choices, 2, difficulty, 0, NULL);

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

static void choose_labyrinth(void){
    if(file_exists(SAVE_PATH, 1) != LABYRINTH_SUCCESS){
        error_msg = "Le dossier de sauvegarde n'est pas accessible ou n'existe pas";
        return;
    }

    char** folder_files = get_folder_content(SAVE_PATH);
    if(folder_files == NULL){
        error_msg = "Impossible de lire le contenu du dossier de sauvegarde";
        return;
    }

    int nb_labyrinths = 0;
    char* extension_start;
    char** labyrinths_list = NULL;
    char** temp = NULL;
    for(int i = 0 ; folder_files[i] != NULL ; i++){
        printf("fichier %s\n", folder_files[i]);
        //on recherche l'extension
        extension_start = strrchr(folder_files[i], '.');
        if(extension_start == NULL){ // pas d'extension = pas de .cfg
            free(folder_files[i]);
            continue;
        }

        //si ce n'est pas un cfg
        if(extension_start[1] != 'c' || extension_start[2] != 'f' || extension_start[3] != 'g'){
            free(folder_files[i]);
            continue;
        }

        temp = realloc(labyrinths_list, (nb_labyrinths+1) * sizeof(char*));
        if(temp == NULL){
            for(int j = 0 ; j < nb_labyrinths ; j++){
                free(labyrinths_list[j]);
            }
            if(labyrinths_list != NULL){
                free(labyrinths_list);
            }
            error_msg = "Erreur d'allocation de mémoire lors de la lecture du dossier de sauvegarde";
            return;
        }
        printf("après allocation de tmp avec l'indice %d\n", i);

        labyrinths_list = temp;

        int name_length = strlen(folder_files[i])-4;

        labyrinths_list[nb_labyrinths] = malloc(name_length+1);
        if(labyrinths_list[nb_labyrinths] == NULL){
            for(int j = 0 ; j < nb_labyrinths ; j++){
                free(labyrinths_list[j]);
            }
            if(labyrinths_list != NULL){
                free(labyrinths_list);
            }
            error_msg = "Erreur d'allocation de mémoire lors de la lecture du dossier de sauvegarde";
            return;
        }

        memcpy(labyrinths_list[nb_labyrinths], folder_files[i], name_length);
        labyrinths_list[nb_labyrinths][name_length] = '\0';
        free(folder_files[i]);
        
        printf("après copie et libération de string avec l'indice %d\n", i);
        nb_labyrinths++;
    }
    free(folder_files);
    printf("après libération de folder_files\n");

    int user_choice = show_labyrinths_list(labyrinths_list, nb_labyrinths);

    if(user_choice == -1){ //Si l'utilisateur veut retourner en arrière
        free(labyrinths_list);
        return;
    }

    if(loaded_labyrinth != NULL){
        free(loaded_labyrinth);
        loaded_labyrinth = NULL;
    }

    LabyrinthStatus labyrinth_load_status = load_labyrinth(labyrinths_list[user_choice], &loaded_labyrinth);
    switch(labyrinth_load_status){
        case LABYRINTH_SUCCESS:
            success_msg = "Labyrinthe chargé avec succès";
            break;

        case LABYRINTH_MEMORY_ERROR:
            error_msg = "Une erreur d'allocation de mémoire empêche le chargement du labyrinthe";
            break;

        case LABYRINTH_CORRUPTED:
            error_msg = "Les paramètres du labyrinthe sont invalides";
            break;

        case LABYRINTH_INVALID_NAME:
            error_msg = "Le nom du labyrinthe est invalide";
            break;

        case LABYRINTH_NOT_FOUND:
            error_msg = "Aucun fichier correspondant à ce labyrinthe n'a été trouvé";
            break;

        case LABYRINTH_PERMISSION_ERROR:
            error_msg = "Une erreur de permission empêche le chargement du labyrinthe";
            break;

        case LABYRINTH_INVALID_FORMAT:
            error_msg = "Le contenu du fichier de sauvegarde est invalide";
            break;
        
        default:
            error_msg = "Une erreur s'est produite";
            break;
    }
}

static int show_labyrinths_list(char** labyrinths_list, int nb_labyrinths){
    int current_choice = 0;
    KeyboardKey key;

    while(1){
        set_background_color(MENU_BACKGROUND_COLOR);
        print_choices(labyrinths_list, nb_labyrinths, current_choice, 0, NULL);

        while((key = read_entry()) == LEFT || key == RIGHT ||key == NONE);

        switch(key){
            case ENTER:
                return current_choice;
            
            case R:
                return -1;

            case UP:
                current_choice--;
                break;
            
            case DOWN:
                current_choice++;
                break;

            default:
                continue;
        }

        if(current_choice > nb_labyrinths-1){
            current_choice = 0;
        }
        else if (current_choice < 0){
            current_choice = nb_labyrinths-1;
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

static void print_choices(char** choices_list, int nb_choices, int current_choice, int nb_disabled_choices, int* disabled_choices){
    if(choices_list == NULL){
        return;
    }

    int disabled;
    for(int i = 0 ; i < nb_choices ; i++){
        if(nb_disabled_choices > 0 && int_array_contains(i, nb_disabled_choices, disabled_choices)){
            disabled = 1;
        }
        else{
            disabled = 0;
        }

        print_text(
            choices_list[i],
            disabled ? MENU_DISABLED_CHOICE_COLOR : MENU_CHOICE_COLOR,
            0,
            current_choice == i ? MENU_SELECTED_CHOICE_COLOR : MENU_BACKGROUND_COLOR,
            1
        );
    }  
}

