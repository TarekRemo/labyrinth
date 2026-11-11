#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"
#include "utils.h"

static void sort_scores_array(long int scores[10], char* player_names[10]);
static int push_score(long int score, long int existing_scores[10]);

LabyrinthStatus save_labyrinth(char* name, Labyrinth* labyrinth){
    if(labyrinth == NULL || labyrinth->cells == NULL){
        return LABYRINTH_CORRUPTED;
    }

    if(!is_valid_labyrinth_params(labyrinth->difficulty, labyrinth->height, labyrinth->width)){
        return LABYRINTH_CORRUPTED;
    }

    if(!is_valid_labyrinth_name(name)){
        return LABYRINTH_INVALID_NAME;
    }

    LabyrinthStatus file_exists_status = file_exists(LABYRINTHS_SAVE_PATH, 1);
    LabyrinthStatus folder_creation_status;
    switch(file_exists_status){
        case LABYRINTH_SUCCESS:
            break;

        case LABYRINTH_NOT_FOUND:
            folder_creation_status = create_folder(LABYRINTHS_SAVE_PATH); 
            if(folder_creation_status != LABYRINTH_SUCCESS){
                return folder_creation_status;
            }
            break;

        default:
            return file_exists_status;
    }

    char* file_path = build_save_file_path(name, 1);
    if(file_path == NULL){
        return LABYRINTH_UNKNOWN;
    }

    //Vérifier qu'un labyrinthe avec ce nom n'existe pas déjà
    if( (file_exists_status = file_exists(file_path, 0)) == LABYRINTH_SUCCESS || file_exists_status == LABYRINTH_PERMISSION_ERROR){
        free(file_path);
        return LABYRINTH_ALREADY_EXISTS;
    }

    FILE* save_file = fopen(file_path, "w");
    if(save_file == NULL){
        free(file_path);
        return LABYRINTH_UNKNOWN;
    }

    /**
     * Format : 
     * 1- Nom du labyrinthe 
     * 2- difficulté (0=easy, 1=hard)
     * 3- Hauteur
     * 4- Largeur
     */
    fprintf(
        save_file,
        "%s\n%d\n%d\n%d\n", 
        name,
        labyrinth->difficulty,
        labyrinth->height,
        labyrinth->width
    );

    for(int line = 0 ; labyrinth->height > line ; line++){
        for(int col = 0 ; labyrinth->width > col ; col++){
            fprintf(save_file, "%c", get_cell_representation(labyrinth->cells[line][col]));
        }
        fprintf(save_file, "\n");
    }

    free(file_path);
    fclose(save_file);

    return LABYRINTH_SUCCESS;
}

LabyrinthStatus load_labyrinth(char* name, Labyrinth** labyrinth){
    *labyrinth = NULL;

    if(!is_valid_labyrinth_name(name)){
        return LABYRINTH_INVALID_NAME;
    }

    char* file_path = malloc(strlen(LABYRINTHS_SAVE_PATH) + strlen(name) + 5);
    strcpy(file_path, LABYRINTHS_SAVE_PATH); 
    strcat(file_path, name);
    strcat(file_path, ".cfg");

    LabyrinthStatus file_exists_status = file_exists(file_path, 0);
    if(file_exists_status != LABYRINTH_SUCCESS){
        free(file_path);
        return file_exists_status;
    }

    FILE* save_file = fopen(file_path, "r");
    free(file_path); //on n'en a plus besoin
    if(save_file == NULL){
        fclose(save_file);
        return LABYRINTH_UNKNOWN;
    }

    //Ignorer la 1ère ligne contenant le nom
    char name_line[MAX_LABYRINTH_NAME_LENGTH + 2];   // nom + '\n' + '\0'
    if(fgets(name_line, sizeof(name_line), save_file) == NULL){
        fclose(save_file);
        return LABYRINTH_INVALID_FORMAT;
    }

    int difficulty, height, width;
    if(
        !read_int(save_file, &difficulty) || 
        !read_int(save_file, &height) ||
        !read_int(save_file, &width) 
    ){
        fclose(save_file);
        return LABYRINTH_INVALID_FORMAT;
    }

    if(!is_valid_labyrinth_params(difficulty, height, width)){
        fclose(save_file);
        return LABYRINTH_CORRUPTED;
    }

    *labyrinth = allocate_labyrinth(difficulty, height, width);
    if(*labyrinth == NULL){
        fclose(save_file);
        return LABYRINTH_MEMORY_ERROR;
    }

    char c;
    Cell content;
    for(int line = 0 ; height > line ; line++){
        for(int col = 0 ; width > col ; col++){
            c=fgetc(save_file);

            //caractères non attendu
            if( (content = get_cell(c)) == UNKNOWN ){
                free_labyrinth(*labyrinth);
                *labyrinth = NULL;
                fclose(save_file);
                return LABYRINTH_INVALID_FORMAT;
            }

            (*labyrinth)->cells[line][col] = content;
        }

        c=fgetc(save_file);

        //retour à la ligne obligatoire à la fin de chaque ligne sauf pour la dernière ligne 
        if(line < height-1){
            if(c != '\n'){
                free_labyrinth(*labyrinth);
                *labyrinth = NULL;
                fclose(save_file);
                return LABYRINTH_INVALID_FORMAT;
            }
        }
        //fin de fichier ou retour à la ligne obligatoire pour la dernière ligne
        else{
            if(c != '\n' && c != EOF && c != '\0'){
                free_labyrinth(*labyrinth);
                *labyrinth = NULL;
                fclose(save_file);
                return LABYRINTH_INVALID_FORMAT;
            }
        }
    }

    return LABYRINTH_SUCCESS;
}

LabyrinthStatus save_score(char* labyrinth_name, char* player_name, long int score){
    if(!is_valid_labyrinth_name(labyrinth_name) || strlen(player_name) > MAX_PLAYER_NAME_LENGTH){
        return LABYRINTH_INVALID_NAME;
    }

    if(labyrinth_name == NULL || score <= 0){
        return LABYRINTH_INVALID_FORMAT;
    }

    long int existing_scores[10];
    char* existing_player_names[10];
    for(int i = 0 ; i < 10 ; i++){
        existing_player_names[i] = malloc(sizeof(char) * MAX_PLAYER_NAME_LENGTH);
        if(existing_player_names[i] == NULL){
            for(int j = 0 ; j < i ; j++){
                free(existing_player_names[j]);
            }
            return LABYRINTH_MEMORY_ERROR;
        }
    }

    LabyrinthStatus existing_scores_status = load_scores(labyrinth_name, existing_scores, existing_player_names);
    if(existing_scores_status == LABYRINTH_NOT_FOUND){
        existing_scores[0] = score;
        strcpy(existing_player_names[0], player_name);

        for(int i = 1 ; i < 10 ; i++){
            existing_scores[i] = -1;
            existing_player_names[i] = NULL;
        }
    }
    else if(existing_scores_status == LABYRINTH_SUCCESS){
        sort_scores_array(existing_scores, existing_player_names);

        int new_score_index = push_score(score, existing_scores);
        if(new_score_index == -1){
            for(int i = 0 ; i < 10 ; i++){
                free(existing_player_names[i]);
            }
            return LABYRINTH_NO_CHANGE;
        }

        strcpy(existing_player_names[new_score_index], player_name);
    }
    else{
        return existing_scores_status;
    }

    char* file_path = build_save_file_path(labyrinth_name, 2);
    if(file_path == NULL){
        for(int i = 0 ; i < 10 ; i++){
            free(existing_player_names[i]);
        }
        return LABYRINTH_UNKNOWN;
    }

    FILE* save_file = fopen(file_path, "w");
    free(file_path);
    if(save_file == NULL){
        for(int i = 0 ; i < 10 ; i++){
            free(existing_player_names[i]);
        }
        return LABYRINTH_UNKNOWN;
    }

    for(int i = 0 ; existing_scores[i] != -1 ; i++){
        fprintf(save_file, "%s %ld\n", existing_player_names[i], existing_scores[i]);
        free(existing_player_names[i]);
    }

    fclose(save_file);
    return LABYRINTH_SUCCESS;
}

/**
 * Permet d'obtenir la liste des scores enregistrés pour un labyrinthe.
 * 
 * @param labyrinth_name le nom du labyrinthe
 * @param scores tableau qui contiendra les scores
 * @param player_names tableau qui contiendra les noms des joueurs de chaque score dans scores.
 * 
 * @return l'état de l'opération      
 */
LabyrinthStatus load_scores(char* labyrinth_name, long int scores[10], char* player_names[10]){
    if(!is_valid_labyrinth_name(labyrinth_name) || scores == NULL || player_names == NULL){
        return LABYRINTH_INVALID_FORMAT;
    }

    char* file_path = build_save_file_path(labyrinth_name, 2);
    if(file_path == NULL){
        return LABYRINTH_MEMORY_ERROR;
    }

    LabyrinthStatus file_exists_status = file_exists(file_path, 0);
    switch(file_exists_status){
        case LABYRINTH_SUCCESS:
            break;
        
        default:
            return file_exists_status;
    }

    FILE* file = fopen(file_path, "r");
    free(file_path);
    if(file == NULL){
        return LABYRINTH_MEMORY_ERROR;
    }

    char score_str[30];
    char player_name[MAX_PLAYER_NAME_LENGTH];
    char* end_ptr; 
    long int score;
    int line = 0;
    int fscanf_result;
    while(1){
        if(line == 10){
            fclose(file);
            return LABYRINTH_SUCCESS;
        }
        
        fscanf_result = fscanf(file, "%s", player_name);
        if(fscanf_result != 1){
            fclose(file);
            //fin de fichier avant de récupérer un nom = succès
            if(fscanf_result == EOF){
                for(int i = 9 ; i >= line ; i--){
                    scores[i] = -1; //pour indiquer le nombre de scores récupérés
                    player_names[i] = NULL;
                }
                return LABYRINTH_SUCCESS;
            }

            return LABYRINTH_INVALID_FORMAT;
        }
        if(strchr(player_name, '\0') == NULL){
            fclose(file);
            return LABYRINTH_INVALID_FORMAT;
        }

        fscanf_result = fscanf(file, "%s", score_str);
        if(fscanf_result != 1){
            fclose(file);
            //nom existe mais pas de score = erreur
            return LABYRINTH_INVALID_FORMAT;
        }
        if(strchr(score_str, '\0') == NULL){
            fclose(file);
            return LABYRINTH_INVALID_FORMAT;
        }

        score = strtol(score_str, &end_ptr, 10);
        if(end_ptr != NULL){ //contient des lettres
            fclose(file);
            return LABYRINTH_INVALID_FORMAT;
        }

        scores[line] = score;
        strcpy(player_names[line], player_name);
        line++;
    }
}

char* build_save_file_path(char* labyrinth_name, int save_type){
    char* save_path = save_type == 1 ? LABYRINTHS_SAVE_PATH  : (save_type == 2 ? SCORES_SAVE_PATH : NULL);
    if(save_path == NULL){
        return NULL;
    }

    char* file_path = malloc(strlen(save_path)+strlen(labyrinth_name)+5);
    if(file_path == NULL){
        return NULL;
    }
    strcpy(file_path, save_path);
    strcat(file_path, labyrinth_name);
    strcat(file_path, ".cfg");

    return file_path;
}

static void sort_scores_array(long int scores[10], char* player_names[10]){
    int temp;
    char* temp_name;
    for (int i = 0; i < 9; ++i){
        for (int j = 0; j < (9 - i); ++j ){
            if (scores[j] > scores[j+1]){
                temp = scores[j+1];
                temp_name = player_names[j+1];

                scores[j+1] = scores[j];
                player_names[j+1] = player_names[j];

                scores[j] = temp;
                player_names[j] = temp_name;
            }
        }
    } 
}

static int push_score(long int score, long int existing_scores[10]){
    int push_back_index = 9;
    for(int i = 0 ; i < 10 ; i++){
        if(existing_scores[i] == -1){
            push_back_index = i;
        }
    }

    for(int i = 0 ; i < 10 ; i++){
        if(existing_scores[i] == -1){
            existing_scores[i] = score;
            return i;
        }
        if(score > existing_scores[i]){
            for(int j = push_back_index ; j > i ; j--){
                existing_scores[j] = existing_scores[j-1];
            }
            existing_scores[i] = score;
            return i;
        }
    }

    return -1;
}
