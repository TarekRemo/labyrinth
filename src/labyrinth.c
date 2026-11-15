#include <stdlib.h>
#include <string.h>
#include "labyrinth.h"

Labyrinth* allocate_labyrinth(char name[MAX_LABYRINTH_NAME_LENGTH+1], int difficulty, int height, int width){
    if(!is_valid_labyrinth_name(name)){
        return NULL;
    }

    Labyrinth* labyrinth = malloc(sizeof(Labyrinth));
    if(labyrinth == NULL){
        return NULL;
    }

    //Allocation des lignes du tableau 2D des cellules
    labyrinth->cells = malloc(height * sizeof(Cell*));
    if(labyrinth->cells == NULL){
        free(labyrinth);
        return NULL;
    }

    //Allocation des colonnes de chaque ligne du tableau 2D des cellules
    for(int line = 0 ; height > line ; line++){
        labyrinth->cells[line] = malloc(width * sizeof(Cell));

        if(labyrinth->cells[line] == NULL){
            for(int i = 0 ; line > i ; i++){
                free(labyrinth->cells[i]);
            }

            free(labyrinth->cells);
            free(labyrinth);

            return NULL;
        }
    }

    strcpy(labyrinth->name, name);
    labyrinth->height = height;
    labyrinth->width = width;
    labyrinth->difficulty = difficulty;
    labyrinth->door_unlocked = 0;
    labyrinth->monster_count = 0;
    labyrinth->monsters = NULL;

    return labyrinth;
}

void free_labyrinth(Labyrinth* labyrinth){
    if(labyrinth != NULL){
        for(int line = 0 ; labyrinth->height > line ; line++){
            free(labyrinth->cells[line]);
        }

        free(labyrinth->cells);
        if(labyrinth->monsters != NULL){
            free(labyrinth->monsters);
        }
        free(labyrinth);
    }
}

void break_random_walls(Labyrinth* labyrinth){
    int height = labyrinth->height;
    int width = labyrinth->width;

    int nb_to_break = BROKEN_WALL_PERCENTAGE * (height-1) * (width-1);

    while(nb_to_break > 0){
        int random_line = rand()%height;
        int random_width = rand()%width;

        int break_wall = labyrinth->cells[random_line][random_width] == WALL &&
                            random_line > 0 && random_width > 0 &&
                            random_line < height -1 && random_width < width-1;

        if(break_wall){
            break_wall = break_wall &&
                        (
                            (
                                labyrinth->cells[random_line-1][random_width] == EMPTY &&
                                labyrinth->cells[random_line+1][random_width] == EMPTY
                            ) ||
                            (
                                labyrinth->cells[random_line][random_width-1] == EMPTY &&
                                labyrinth->cells[random_line+1][random_width+1] == EMPTY
                            )
                        );
        }

        if(break_wall){
            labyrinth->cells[random_line][random_width] = EMPTY;
            nb_to_break--;
        }
    }
}

void add_random_objects(Labyrinth* labyrinth){
    int height = labyrinth->height;
    int width = labyrinth->width;

    int random_line;
    int random_col;

    do{
        random_line = rand()%height;
        random_col = rand()%width;
    }while(labyrinth->cells[random_line][random_col] != EMPTY);
    labyrinth->cells[random_line][random_col] = KEY;

    int nb_traps_to_add = (height*width)*0.03; 
    while(nb_traps_to_add > 0){
        do{
            random_line = rand()%height;
            random_col = rand()%width;
        }while(labyrinth->cells[random_line][random_col] != EMPTY);
        labyrinth->cells[random_line][random_col] = TRAP;
        nb_traps_to_add--;
    }

    int nb_treasures_to_add = (height*width)*0.02; 
    while(nb_treasures_to_add > 0){
        do{
            random_line = rand()%height;
            random_col = rand()%width;
        }while(labyrinth->cells[random_line][random_col] != EMPTY);
        labyrinth->cells[random_line][random_col] = TREASURE;
        nb_treasures_to_add--;
    }
}

int is_valid_labyrinth_params(Difficulty difficulty, int height, int width){
    return height >= 3 && 
           width >= 3  &&
           height%2 == 1 && 
           width%2 == 1 &&
           (difficulty == EASY || difficulty == HARD);
}

int is_valid_labyrinth_name(char* name){
    if(name == NULL){
        return 0;
    }

    size_t length = strlen(name);
    if(length == 0 || length > MAX_LABYRINTH_NAME_LENGTH){
        return 0;
    }

    for(size_t i = 0 ; length > i ; i++){
        char c = name[i];
        int is_allowed = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
            || c == '_' || c == '-';
        if(!is_allowed){
            return 0;
        }
    }

    return 1;
}

char get_cell_representation(Cell content){
    switch (content){
        case EMPTY:
            return ' ';

        case WALL:
            return '#';

        case DOOR:
            return '_';

        case KEY:
            return '^';

        case TREASURE:
            return '$';

        case TRAP:
            return 'X';

        case PLAYER:
            return 'O';

        default:
            return 'E'; //erreur
    }
}

Cell get_cell(char c){
    switch (c){
        case ' ':
            return EMPTY;

        case '#':
            return WALL;

        case '_':
            return DOOR;

        case '^':
            return KEY;

        case '$':
            return TREASURE;

        case 'X':
            return TRAP;

        case 'O':
            return PLAYER;

        default:
            return UNKNOWN;
    }
}