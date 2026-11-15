#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdlib.h>
#include "labyrinth.h"

/**
 * Construit un labyrinthe à partir d'un plan texte :
 * '#' = mur, '.' = case vide, '$' = trésor, 'O' = joueur, '^' = clé, 'X' = piège, '_' = porte.
 * Le labyrinthe retourné doit être libéré avec free_labyrinth().
 */
static inline Labyrinth* build_labyrinth_from_map(const char** map, int height, int width){
    char name[MAX_LABYRINTH_NAME_LENGTH + 1] = "test";
    Labyrinth* labyrinth = allocate_labyrinth(name, EASY, height, width);
    if(labyrinth == NULL){
        return NULL;
    }

    for(int line = 0 ; line < height ; line++){
        for(int col = 0 ; col < width ; col++){
            char c = map[line][col];
            labyrinth->cells[line][col] = c == '.' ? EMPTY : get_cell(c);
        }
    }
    return labyrinth;
}

/**
 * Compte le nombre de cellules d'un type donné.
 */
static inline int count_cells(Labyrinth* labyrinth, Cell type){
    int count = 0;
    for(int line = 0 ; line < labyrinth->height ; line++){
        for(int col = 0 ; col < labyrinth->width ; col++){
            if(labyrinth->cells[line][col] == type){
                count++;
            }
        }
    }
    return count;
}

#endif
