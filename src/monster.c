#include <stdlib.h>
#include <stdio.h>
#include "monster.h"

LabyrinthStatus add_random_monsters(Labyrinth* labyrinth){
    int nb_cells = labyrinth->width * labyrinth->height;
    int nb_ghosts = (nb_cells/CELLS_PER_GHOST);
    int nb_ogres = (nb_cells/CELLS_PER_OGRE);
    if(nb_ogres + nb_ghosts == 0){ //Le labyrinthe n'est pas assez grand pour avoir des monstres
        return LABYRINTH_NO_CHANGE;
    }

    int available_count = 0;
    for(int line = 0 ; line < labyrinth->height ; line++){
        for(int col = 0 ; col < labyrinth->width ; col++){
            if(labyrinth->cells[line][col] == EMPTY){
                available_count++;
            }
        }
    }
    labyrinth->monster_count = available_count < (nb_ghosts + nb_ogres) ? available_count : (nb_ghosts + nb_ogres);

    labyrinth->monsters = malloc(labyrinth->monster_count * sizeof(Monster));
    if(labyrinth->monsters == NULL){
        return LABYRINTH_MEMORY_ERROR;
    }

    for(int i = 0 ; i < labyrinth->monster_count ; i++){
        while(1){
            int random_line = rand() % labyrinth->height;
            int random_col = rand() % labyrinth->width;

            if(labyrinth->cells[random_line][random_col] != EMPTY){
                continue;
            }

            int already_used = 0;
            for(int monster_index = 0 ; monster_index < i ; monster_index++){
                if(labyrinth->monsters[monster_index].line == random_line &&
                    labyrinth->monsters[monster_index].col == random_col){
                    already_used = 1;
                    break;
                }
            }
            if(already_used){
                continue;
            }
            
            int is_ghost = (nb_ghosts >= nb_ogres);

            labyrinth->monsters[i].line = random_line;
            labyrinth->monsters[i].col = random_col;
            labyrinth->monsters[i].penalty_points = is_ghost ? GHOST_PENALITY : OGRE_PENALITY;
            labyrinth->monsters[i].representation = is_ghost ? GHOST_REPRESENTATION : OGRE_REPRESENTATION;
            labyrinth->monsters[i].mobility = is_ghost ? move_ghost : move_ogre;

            if(is_ghost){
                nb_ghosts--;
            }
            else{
                nb_ogres--;
            }

            break;
        }
    }
    return LABYRINTH_SUCCESS;
}

void move_ghost(Labyrinth* labyrinth, Monster* monster){
    int candidate_count = 0;
    int selected_line = monster->line;
    int selected_col = monster->col;

    for(int line = 0 ; line < labyrinth->height ; line++){
        for(int col = 0 ; col < labyrinth->width ; col++){
            int distance = abs(line - monster->line) + abs(col - monster->col);
            if(distance > monster->penalty_points ||
               labyrinth->cells[line][col] == WALL ||
               (labyrinth->cells[line][col] == DOOR && labyrinth->door_unlocked == 0)){
                continue;
            }

            int occupied = 0;
            for(int i = 0 ; i < labyrinth->monster_count ; i++){
                if(&labyrinth->monsters[i] != monster &&
                   labyrinth->monsters[i].line == line &&
                   labyrinth->monsters[i].col == col){
                    occupied = 1;
                    break;
                }
            }
            if(occupied){
                continue;
            }

            candidate_count++;
            if(rand() % candidate_count == 0){
                selected_line = line;
                selected_col = col;
            }
        }
    }

    monster->line = selected_line;
    monster->col = selected_col;
}

void move_ogre(Labyrinth* labyrinth, Monster* monster){
    static const int line_offsets[] = {-1, 1, 0, 0};
    static const int col_offsets[] = {0, 0, -1, 1};

    for(int step = 0 ; step < monster->penalty_points ; step++){
        int target_line = -1;
        int target_col = -1;
        int current_treasure_distance = labyrinth->height + labyrinth->width;
        int selected_line = monster->line;
        int selected_col = monster->col;
        int candidate_count = 0;

        for(int line = 0 ; line < labyrinth->height ; line++){
            for(int col = 0 ; col < labyrinth->width ; col++){
                if(labyrinth->cells[line][col] == TREASURE){
                    int distance = abs(line - monster->line) + abs(col - monster->col);
                    if(distance < current_treasure_distance){
                        current_treasure_distance = distance;
                        target_line = line;
                        target_col = col;
                    }
                }
            }
        }

        if(target_line != -1 && current_treasure_distance <= 1){
            break;
        }

        for(int direction = 0 ; direction < 4 ; direction++){
            int new_line = monster->line + line_offsets[direction];
            int new_col = monster->col + col_offsets[direction];
            if(new_line < 0 || new_line >= labyrinth->height ||
               new_col < 0 || new_col >= labyrinth->width ||
               labyrinth->cells[new_line][new_col] == WALL ||
               labyrinth->cells[new_line][new_col] == TREASURE ||
               (labyrinth->cells[new_line][new_col] == DOOR && labyrinth->door_unlocked == 0)){
                continue;
            }

            int occupied = 0;
            for(int i = 0 ; i < labyrinth->monster_count ; i++){
                if(&labyrinth->monsters[i] != monster &&
                   labyrinth->monsters[i].line == new_line &&
                   labyrinth->monsters[i].col == new_col){
                    occupied = 1;
                    break;
                }
            }
            if(occupied){
                continue;
            }

            if(target_line == -1){
                candidate_count++;
                if(rand() % candidate_count == 0){
                    selected_line = new_line;
                    selected_col = new_col;
                }
            }
            else{
                int distance = abs(target_line - new_line) + abs(target_col - new_col);
                int selected_distance = abs(target_line - selected_line) + abs(target_col - selected_col);
                if(distance < selected_distance){
                    selected_line = new_line;
                    selected_col = new_col;
                    candidate_count = 1;
                }
                else if(distance == selected_distance && rand() % ++candidate_count == 0){
                    selected_line = new_line;
                    selected_col = new_col;
                }
            }
        }

        if(selected_line == monster->line && selected_col == monster->col){
            break;
        }
        monster->line = selected_line;
        monster->col = selected_col;
    }
}