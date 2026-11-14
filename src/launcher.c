#include <stdlib.h>
#include <string.h>
#include "launcher.h"
#include "generator.h"
#include "io.h"

#define MAX_MENU_MSG_LENGTH 150

Labyrinth* loaded_labyrinth;

char* error_msg;
char* success_msg;

GAMESTATUS current_game_status;

long int score;

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
 * Permet de lancer une partie avec le labyrinthe chargé dans loaded_laburinth.
 * 
 * @return void
 */
static void play(void);

/**
 * gére l'enregistrement du score actuel (variable globale score)
 * 
 * @return void
 */
static void handle_score_saving(void);

/**
 * Affiche les meilleurs scores du labyrinthe chargé en mémoire.
 *
 * @return void
 */
static void show_high_scores(void);

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

    current_game_status = NOT_STARTED;
    loaded_labyrinth = NULL;

    MenuChoice choice;
    while(1){
        choice = show_menu();
        error_msg = NULL;
        success_msg = NULL;

        switch(choice){
            case PLAY:
                play();
                break;

            case CREATE_LABYRINTH:
                create_labyrinth();
                break;

            case LOAD_LABYRINTH:
                choose_labyrinth();
                break;

            case VIEW_SCORES:
                show_high_scores();
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

    if(current_game_status == WON){
        free_labyrinth(loaded_labyrinth);
        loaded_labyrinth = NULL;
    }

    int labyrinth_is_loaded = loaded_labyrinth != NULL ? 1 : 0;

    int scores_available = 0;
    if(labyrinth_is_loaded){
        long int scores[MAX_SCORES];
        char player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1];
        LabyrinthStatus scores_status = load_scores(loaded_labyrinth->name, scores, player_names);
        if(scores_status == LABYRINTH_SUCCESS){
            scores_available = scores[0] != -1;
        }
        else if(scores_status != LABYRINTH_NOT_FOUND){
            error_msg = "Impossible de lire les meilleurs scores de ce labyrinthe";
        }
    }

    char* menu_choices[5];
    MenuChoice menu_choice_values[5];
    int nb_choices = 0;
    menu_choices[nb_choices] = current_game_status == NOT_FINISHED ? "Reprendre" : "Jouer";
    menu_choice_values[nb_choices++] = PLAY;
    menu_choices[nb_choices] = "Créer un labyrinthe";
    menu_choice_values[nb_choices++] = CREATE_LABYRINTH;
    menu_choices[nb_choices] = "Charger un labyrinthe";
    menu_choice_values[nb_choices++] = LOAD_LABYRINTH;
    if(scores_available){
        menu_choices[nb_choices] = "Voir les meilleurs scores de ce labyrinthe";
        menu_choice_values[nb_choices++] = VIEW_SCORES;
    }
    menu_choices[nb_choices] = "Quitter";
    menu_choice_values[nb_choices++] = QUIT;

    int current_choice = labyrinth_is_loaded ? 0 : 1;
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
                1,
                1
            );
        }

        if(success_msg != NULL){
            print_text(
                success_msg,
                MENU_SUCCESS_TEXT_COLOR,
                0,
                MENU_BACKGROUND_COLOR,
                1,
                1
            );
        }

        char score_msg[60];
        if(current_game_status == WON){
            sprintf(score_msg, "\nBRAVO! Vous avez gagné la partie.\nVotre score final est %ld", score);
            print_text(
                score_msg,
                MENU_GAME_RESULT_TEXT_COLOR,
                0,
                MENU_BACKGROUND_COLOR,
                1,
                1
            );
        }
        else if (current_game_status == NOT_FINISHED){
            sprintf(score_msg, "\nVotre score actuel est %ld", score);
            print_text(
                score_msg,
                MENU_GAME_RESULT_TEXT_COLOR,
                0,
                MENU_BACKGROUND_COLOR,
                1,
                1
            );
        }

        labyrinth_is_loaded = loaded_labyrinth != NULL ? 1 : 0;
        print_choices(menu_choices, nb_choices, current_choice, labyrinth_is_loaded ? 0 : 1, labyrinth_is_loaded ? NULL : disabled_choices);

        //On attend une entrée reconnue
        while(
            (key=read_entry()) == NONE ||
            key == LEFT || key == RIGHT ||
            key == Q || key == D
        );

        switch(key){
            case UP:
            case DOWN:
            case Z:
            case S:
                modifier = (key == UP || key == Z) ? -1 : 1;
                current_choice += modifier;
                if(current_choice >= nb_choices){
                    current_choice = 0;
                }
                else if(current_choice < 0){
                    current_choice = nb_choices - 1;
                }
                while(!labyrinth_is_loaded && menu_choice_values[current_choice] == PLAY){
                    current_choice += modifier;
                    if(current_choice >= nb_choices){
                        current_choice = 0;
                    }
                    else if(current_choice < 0){
                        current_choice = nb_choices - 1;
                    }
                }
                continue;

            case ENTER:
                set_background_color(MENU_BACKGROUND_COLOR);
                return menu_choice_values[current_choice];

            case R:
                return RETURN;

            default:
                continue;
        }
    }
}

static void play(){
    if(loaded_labyrinth == NULL){
        error_msg = "Aucun labyrinthe n'est chargé en mémoire pour jouer";
        return;
    }
    
    //Temps écoulé depuis le début de la partie
    static long int nb_mvmts;

    if(current_game_status == NOT_STARTED){ //réinitialisation en début de partie
        nb_mvmts = 0;
        score = 0;
    }

    current_game_status = launch_game(loaded_labyrinth, &nb_mvmts, &score);
    if(current_game_status == WON && score > 0){ //enregistrer le score du joueur
        handle_score_saving();
    }
}

static void handle_score_saving(){
    long int current_scores[MAX_SCORES];
    char current_player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1];

    LabyrinthStatus load_scores_status = load_scores(loaded_labyrinth->name, current_scores, current_player_names);
    if(load_scores_status == LABYRINTH_SUCCESS || load_scores_status == LABYRINTH_NOT_FOUND){
        //vérifier si le score fait partie des meilleurs scores enregistrés
        if(load_scores_status == LABYRINTH_SUCCESS){
            int count = 0;
            while(count < MAX_SCORES && current_scores[count] != -1){
                count++;
            }
            if(count == MAX_SCORES && score <= current_scores[MAX_SCORES - 1]){
                return; //le score ne fait pas partie des meilleurs scores
            }
        }
        
        set_background_color(MENU_BACKGROUND_COLOR);

        char name[MAX_PLAYER_NAME_LENGTH+1];
        char display_msg[300];
        sprintf(display_msg, "Bravo! Votre score fait partie des %d meilleurs scores.\nEntrer votre nom pour l'enregistrer:", MAX_SCORES);
        print_text(
            display_msg,
            MENU_CHOICE_COLOR,
            0,
            MENU_BACKGROUND_COLOR,
            1,
            1
        );

        set_canonical_mode(1);
        while((read_string(name, sizeof(name))) == 0 || !is_valid_player_name(name)){
            sprintf(display_msg, "Veulliez entrer un nom qui fait moins de %d caractères et ne contient pas d'espaces", MAX_PLAYER_NAME_LENGTH);
            print_text(
                display_msg,
                MENU_ERROR_TEXT_COLOR,
                0,
                MENU_BACKGROUND_COLOR,
                1,
                1
            );
        }
        set_canonical_mode(0);

        LabyrinthStatus save_score_status = save_score(loaded_labyrinth->name, name, score);
        if(save_score_status == LABYRINTH_SUCCESS){
            success_msg = "Votre score a bien été enregistré !";
        }
        else if(save_score_status == LABYRINTH_NO_CHANGE){
            error_msg = "Le fichier des scores n'a pas été modifié: Votre score existait déjà";
        }
        else{
            error_msg = "Echec de l'enregistrement du score";
        }
    }
    else{
        error_msg = "Echec de l'enregistrement du score: Impossible de lire le fichier des scores de ce labyrinthe";
    }
}

static void show_high_scores(){
    long int scores[MAX_SCORES];
    char player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1];
    LabyrinthStatus status = load_scores(loaded_labyrinth->name, scores, player_names);
    if(status != LABYRINTH_SUCCESS){
        error_msg = "Impossible de lire les meilleurs scores de ce labyrinthe";
        return;
    }

    set_background_color(MENU_BACKGROUND_COLOR);
    char title[MAX_LABYRINTH_NAME_LENGTH + 40];
    sprintf(title, "Meilleurs scores du labyrinthe %s", loaded_labyrinth->name);
    print_text(title, MENU_CHOICE_COLOR, 0, MENU_BACKGROUND_COLOR, 1, 1);

    for(int i = 0 ; i < MAX_SCORES && scores[i] != -1 ; i++){
        char score_line[MAX_PLAYER_NAME_LENGTH + 40];
        sprintf(score_line, "%d. %s - %ld", i + 1, player_names[i], scores[i]);
        print_text(score_line, MENU_CHOICE_COLOR, 0, MENU_BACKGROUND_COLOR, 0, 1);
    }

    print_text("Appuyez sur R pour revenir au menu", MENU_CHOICE_COLOR, 0, MENU_BACKGROUND_COLOR, 0, 1);
    KeyboardKey input;
    while((input = read_entry()) != R);
}

static void create_labyrinth(){
    if(loaded_labyrinth != NULL){
        free(loaded_labyrinth);
        loaded_labyrinth = NULL;
    }
    current_game_status = NOT_STARTED;

    set_background_color(MENU_BACKGROUND_COLOR);
    set_canonical_mode(1);

    char name[MAX_LABYRINTH_NAME_LENGTH+1];
    print_text(
        "Entrer le nom du labyrinthe à créer",
        MENU_CHOICE_COLOR,
        0,
        MENU_BACKGROUND_COLOR,
        1,
        1
    );
    while((read_string(name, sizeof(name))) == 0 || !is_valid_labyrinth_name(name)){
        char name_error_msg[250];
        sprintf(name_error_msg, "Veulliez entrer un nom valide qui fait moins de %d caractères", MAX_LABYRINTH_NAME_LENGTH);
        print_text(
            name_error_msg,
            MENU_ERROR_TEXT_COLOR,
            0,
            MENU_BACKGROUND_COLOR,
            1,
            1
        );
    }    
    
    set_background_color(MENU_BACKGROUND_COLOR);

    int height, width;
    Difficulty difficulty;
    print_text(
        "Entrer la hauteur et la largeur du labyrinthe à créer",
        MENU_CHOICE_COLOR,
        0,
        MENU_BACKGROUND_COLOR,
        1,
        1
    );

    while(
        read_int(stdin, &height) == 0 ||
        read_int(stdin, &width) == 0 ||
        height%2 == 0 ||
        width%2 == 0
    ){
        set_background_color(MENU_BACKGROUND_COLOR);
        print_text(
            "Veillez entrer des entiers valides et impairs",
            MENU_ERROR_TEXT_COLOR,
            0,
            MENU_BACKGROUND_COLOR,
            1,
            1
        );
    }

    difficulty = read_labyrinth_difficulty();

    LabyrinthStatus generate_status = generate_labyrinth(&loaded_labyrinth, name, difficulty, height, width);
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
        while(
            (key=read_entry()) == NONE ||
            key == LEFT || key == RIGHT ||
            key == Q || key == D ||
            key == R
        );

        switch(key){
            case UP:
            case DOWN:
            case Z:
            case S:
                difficulty = difficulty == EASY ? HARD : EASY;
                break;

            case ENTER:
                set_background_color(MENU_BACKGROUND_COLOR);
                return difficulty;

            default:
                continue;
        }
    }
}

static void choose_labyrinth(void){
    if(file_exists(LABYRINTHS_SAVE_PATH, 1) != LABYRINTH_SUCCESS){
        error_msg = "Le dossier de sauvegarde n'est pas accessible ou n'existe pas";
        return;
    }

    char** folder_files = get_folder_content(LABYRINTHS_SAVE_PATH);
    if(folder_files == NULL){
        error_msg = "Impossible de lire le contenu du dossier de sauvegarde";
        return;
    }

    int nb_labyrinths = 0;
    char* extension_start;
    char** labyrinths_list = NULL;
    char** temp = NULL;
    for(int i = 0 ; folder_files[i] != NULL ; i++){
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
        
        nb_labyrinths++;
    }
    free(folder_files);

    if(nb_labyrinths == 0){
        error_msg = "Aucun labyrinthe n'a encore été sauvegardé";
        return;
    }
    
    int user_choice = show_labyrinths_list(labyrinths_list, nb_labyrinths);

    if(user_choice == -1){ //Si l'utilisateur veut retourner en arrière
        free(labyrinths_list);
        return;
    }

    if(loaded_labyrinth != NULL){
        free(loaded_labyrinth);
        loaded_labyrinth = NULL;
    }
    current_game_status = NOT_STARTED;

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

        while(
            (key = read_entry()) == LEFT ||
            key == RIGHT || key == D ||
            key == NONE || key == Q
        );

        switch(key){
            case ENTER:
                return current_choice;
            
            case R:
                return -1;

            case UP:
            case Z:
                current_choice--;
                break;
            
            case DOWN:
            case S:
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
            1,
            1
        );
    }  
}

