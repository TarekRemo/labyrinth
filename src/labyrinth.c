#include <stdlib.h>
#include "labyrinth.h"

Labyrinth* allocate_labyrinth(int difficulty, int height, int width){
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

    labyrinth->height = height;
    labyrinth->width = width;
    labyrinth->difficulty = difficulty;

    return labyrinth;
}

void free_labyrinth(Labyrinth* labyrinth){
    if(labyrinth != NULL){
        for(int line = 0 ; labyrinth->height > line ; line++){
            free(labyrinth->cells[line]);
        }

        free(labyrinth->cells);
        free(labyrinth);
    }
}

int is_valid_labyrinth_params(Difficulty difficulty, int height, int width){
    return height >= 3 && 
           width >= 3  &&
           height%2 == 1 && 
           width%2 == 1 &&
           (difficulty == EASY || difficulty == HARD);
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