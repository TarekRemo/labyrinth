#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "io.h"
#include "utils.h"

/**
 * Vérifie qu'un nom de joueur est non vide, ne dépasse pas la longueur maximale
 * et ne contient aucun espace.
 *
 * @param player_name le nom à vérifier.
 *
 * @return 1 si le nom est valide, 0 sinon.
 */
static int is_valid_player_name(const char* player_name);

/**
 * Analyse une ligne de score et extrait le nom du joueur et son score.
 *
 * @param line la ligne contenant un nom suivi d'un score positif.
 * @param player_name le tampon qui recevra le nom extrait; il doit pouvoir
 *        contenir MAX_PLAYER_NAME_LENGTH + 1 caractères.
 * @param score le pointeur qui recevra le score extrait.
 *
 * @return LABYRINTH_SUCCESS si la ligne est valide, LABYRINTH_INVALID_FORMAT sinon.
 */
static LabyrinthStatus parse_score_line(char* line, char* player_name, long int* score);

/**
 * Trie les scores par ordre décroissant en conservant l'association entre
 * chaque score et le nom du joueur correspondant.
 *
 * @param scores tableau de MAX_SCORES scores, les cases inutilisées valant -1.
 * @param player_names noms associés aux scores.
 *
 * @return void
 */
static void sort_scores_array(
    long int scores[MAX_SCORES],
    char player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1]
);

/**
 * Insère un score à sa place dans le classement, en écartant le dernier score
 * si le tableau est déjà complet.
 *
 * @param score le score à insérer.
 * @param scores tableau de MAX_SCORES scores déjà classés par ordre décroissant.
 * @param player_names noms associés aux scores.
 * @param player_name le nom du joueur associé au nouveau score.
 *
 * @return l'indice d'insertion, ou -1 si le score ne rentre pas dans le classement.
 */
static int push_score(
    long int score,
    long int scores[MAX_SCORES],
    char player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1],
    const char* player_name
);

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
    if(!is_valid_labyrinth_name(labyrinth_name)){
        return LABYRINTH_INVALID_NAME;
    }

    if(!is_valid_player_name(player_name)){
        return LABYRINTH_INVALID_NAME;
    }

    if(score <= 0){
        return LABYRINTH_INVALID_FORMAT;
    }

    long int existing_scores[MAX_SCORES];
    char existing_player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1];
    LabyrinthStatus status = load_scores(labyrinth_name, existing_scores, existing_player_names);
    if(status != LABYRINTH_SUCCESS && status != LABYRINTH_NOT_FOUND){
        return status;
    }

    if(push_score(score, existing_scores, existing_player_names, player_name) == -1){
        return LABYRINTH_NO_CHANGE;
    }

    status = file_exists(SCORES_SAVE_PATH, 1);
    if(status == LABYRINTH_NOT_FOUND){
        status = create_folder(SCORES_SAVE_PATH);
    }
    if(status != LABYRINTH_SUCCESS){
        return status;
    }

    char* file_path = build_save_file_path(labyrinth_name, 2);
    if(file_path == NULL){
        return LABYRINTH_MEMORY_ERROR;
    }

    FILE* save_file = fopen(file_path, "w");
    free(file_path);
    if(save_file == NULL){
        return LABYRINTH_UNKNOWN;
    }

    int write_failed = 0;
    for(int i = 0 ; i < MAX_SCORES && existing_scores[i] != -1 ; i++){
        if(fprintf(save_file, "%s %ld\n", existing_player_names[i], existing_scores[i]) < 0){
            write_failed = 1;
            break;
        }
    }

    if(fclose(save_file) != 0){
        write_failed = 1;
    }
    return write_failed ? LABYRINTH_UNKNOWN : LABYRINTH_SUCCESS;
}

LabyrinthStatus load_scores(
    char* labyrinth_name,
    long int scores[MAX_SCORES],
    char player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1]
){
    if(!is_valid_labyrinth_name(labyrinth_name) || scores == NULL || player_names == NULL){
        return LABYRINTH_INVALID_FORMAT;
    }

    for(int i = 0 ; i < MAX_SCORES ; i++){
        scores[i] = -1;
        player_names[i][0] = '\0';
    }

    long int loaded_scores[MAX_SCORES];
    char loaded_player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1] = {{0}};
    for(int i = 0 ; i < MAX_SCORES ; i++){
        loaded_scores[i] = -1;
    }

    char* file_path = build_save_file_path(labyrinth_name, 2);
    if(file_path == NULL){
        return LABYRINTH_MEMORY_ERROR;
    }

    LabyrinthStatus file_exists_status = file_exists(file_path, 0);
    if(file_exists_status != LABYRINTH_SUCCESS){
        free(file_path);
        return file_exists_status;
    }

    FILE* file = fopen(file_path, "r");
    free(file_path);
    if(file == NULL){
        return LABYRINTH_UNKNOWN;
    }

    char line[MAX_PLAYER_NAME_LENGTH + 32];
    LabyrinthStatus status = LABYRINTH_SUCCESS;
    int count = 0;
    while(count < MAX_SCORES && fgets(line, sizeof(line), file) != NULL){
        if(strchr(line, '\n') == NULL && !feof(file)){
            status = LABYRINTH_INVALID_FORMAT;
            break;
        }
        status = parse_score_line(line, loaded_player_names[count], &loaded_scores[count]);
        if(status != LABYRINTH_SUCCESS){
            break;
        }
        count++;
    }

    if(ferror(file)){
        status = LABYRINTH_UNKNOWN;
    }
    if(fclose(file) != 0 && status == LABYRINTH_SUCCESS){
        status = LABYRINTH_UNKNOWN;
    }
    if(status != LABYRINTH_SUCCESS){
        return status;
    }

    sort_scores_array(loaded_scores, loaded_player_names);
    memcpy(scores, loaded_scores, sizeof(loaded_scores));
    memcpy(player_names, loaded_player_names, sizeof(loaded_player_names));
    return LABYRINTH_SUCCESS;
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

static int is_valid_player_name(const char* player_name){
    if(player_name == NULL || player_name[0] == '\0'){
        return 0;
    }

    size_t length = strlen(player_name);
    if(length > MAX_PLAYER_NAME_LENGTH){
        return 0;
    }
    for(size_t i = 0 ; i < length ; i++){
        if(isspace((unsigned char)player_name[i])){
            return 0;
        }
    }
    return 1;
}

static LabyrinthStatus parse_score_line(char* line, char* player_name, long int* score){
    char* cursor = line;
    while(isspace((unsigned char)*cursor)){
        cursor++;
    }

    char* name_start = cursor;
    while(*cursor != '\0' && !isspace((unsigned char)*cursor)){
        cursor++;
    }
    size_t name_length = (size_t)(cursor - name_start);
    if(name_length == 0 || name_length > MAX_PLAYER_NAME_LENGTH){
        return LABYRINTH_INVALID_FORMAT;
    }
    memcpy(player_name, name_start, name_length);
    player_name[name_length] = '\0';

    while(isspace((unsigned char)*cursor)){
        cursor++;
    }
    if(*cursor == '\0'){
        return LABYRINTH_INVALID_FORMAT;
    }

    errno = 0;
    char* end_ptr;
    long int parsed_score = strtol(cursor, &end_ptr, 10);
    if(cursor == end_ptr || errno == ERANGE || parsed_score <= 0){
        return LABYRINTH_INVALID_FORMAT;
    }
    while(isspace((unsigned char)*end_ptr)){
        end_ptr++;
    }
    if(*end_ptr != '\0'){
        return LABYRINTH_INVALID_FORMAT;
    }

    *score = parsed_score;
    return LABYRINTH_SUCCESS;
}

static void sort_scores_array(
    long int scores[MAX_SCORES],
    char player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1]
){
    for(int i = 0 ; i < MAX_SCORES - 1 ; i++){
        for(int j = 0 ; j < MAX_SCORES - i - 1 ; j++){
            if(scores[j] < scores[j + 1]){
                long int temp_score = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp_score;

                char temp_name[MAX_PLAYER_NAME_LENGTH + 1];
                memcpy(temp_name, player_names[j], sizeof(temp_name));
                memcpy(player_names[j], player_names[j + 1], sizeof(temp_name));
                memcpy(player_names[j + 1], temp_name, sizeof(temp_name));
            }
        }
    }
}

static int push_score(
    long int score,
    long int scores[MAX_SCORES],
    char player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1],
    const char* player_name
){
    int count = 0;
    while(count < MAX_SCORES && scores[count] != -1){
        count++;
    }
    if(count == MAX_SCORES && score <= scores[MAX_SCORES - 1]){
        return -1;
    }

    int index = 0;
    while(index < count && score <= scores[index]){
        index++;
    }
    int last_index = count < MAX_SCORES ? count : MAX_SCORES - 1;
    for(int i = last_index ; i > index ; i--){
        scores[i] = scores[i - 1];
        memcpy(player_names[i], player_names[i - 1], MAX_PLAYER_NAME_LENGTH + 1);
    }
    scores[index] = score;
    strcpy(player_names[index], player_name);
    return index;
}
