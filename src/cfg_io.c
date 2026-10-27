#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfg_io.h"
#include "utils.h"

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

    LabyrinthStatus file_exists_status = file_exists(SAVE_PATH, 1);
    switch(file_exists_status){
        case LABYRINTH_SUCCESS:
            break;

        case LABYRINTH_NOT_FOUND:
            LabyrinthStatus folder_creation_status = create_folder(SAVE_PATH); 
            if(folder_creation_status != LABYRINTH_SUCCESS){
                return folder_creation_status;
            }
            break;

        default:
            return file_exists_status;
    }

    char* file_path = build_file_path(name);
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

    char* file_path = malloc(strlen(SAVE_PATH) + strlen(name) + 5);
    strcpy(file_path, SAVE_PATH); 
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

char* build_file_path(char* labyrinth_name){
    char* file_path = malloc(strlen(SAVE_PATH)+strlen(labyrinth_name)+5);
    if(file_path == NULL){
        return NULL;
    }
    strcpy(file_path, SAVE_PATH);
    strcat(file_path, labyrinth_name);
    strcat(file_path, ".cfg");

    return file_path;
}
