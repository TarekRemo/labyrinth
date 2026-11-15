#include <stdlib.h>
#include <stdio.h>
#include "monster.h"

static int monster_can_enter_cell(Labyrinth* labyrinth, Monster* monster, int line, int col){
    if(line < 0 || line >= labyrinth->height ||
       col < 0 || col >= labyrinth->width ||
       labyrinth->cells[line][col] == WALL ||
       labyrinth->cells[line][col] == TRAP ||
       labyrinth->cells[line][col] == TREASURE ||
       (labyrinth->cells[line][col] == DOOR)){
        return 0;
    }

    for(int i = 0 ; i < labyrinth->monster_count ; i++){
        if(&labyrinth->monsters[i] != monster &&
           labyrinth->monsters[i].line == line &&
           labyrinth->monsters[i].col == col){
            return 0;
        }
    }

    return 1;
}

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
            labyrinth->monsters[i].patrol_treasure_line = -1;
            labyrinth->monsters[i].patrol_treasure_col = -1;

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
               !monster_can_enter_cell(labyrinth, monster, line, col)){
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
    // Parcours horaire autour du trésor, en commençant par la case au-dessus.
    static const int patrol_line_offsets[] = {-1, -1, 0, 1, 1, 1, 0, -1};
    static const int patrol_col_offsets[] = {0, 1, 1, 1, 0, -1, -1, -1};

    for(int step = 0 ; step < monster->penalty_points ; step++){
        int target_line = monster->patrol_treasure_line;
        int target_col = monster->patrol_treasure_col;
        if(target_line < 0 || target_line >= labyrinth->height ||
           target_col < 0 || target_col >= labyrinth->width ||
           labyrinth->cells[target_line][target_col] != TREASURE){
            target_line = -1;
            target_col = -1;
            int current_treasure_distance = labyrinth->height + labyrinth->width;

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

            monster->patrol_treasure_line = target_line;
            monster->patrol_treasure_col = target_col;
        }

        int selected_line = monster->line;
        int selected_col = monster->col;
        int candidate_count = 0;

        if(target_line == -1){
            monster->patrol_treasure_line = -1;
            monster->patrol_treasure_col = -1;
        }

        int line_distance = monster->line - target_line;
        int col_distance = monster->col - target_col;
        if(target_line != -1 && abs(line_distance) <= 1 && abs(col_distance) <= 1){
            for(int direction = 0 ; direction < 8 ; direction++){
                if(monster->line == target_line + patrol_line_offsets[direction] &&
                   monster->col == target_col + patrol_col_offsets[direction]){
                    int next_direction = (direction + 1) % 8;
                    int new_line = target_line + patrol_line_offsets[next_direction];
                    int new_col = target_col + patrol_col_offsets[next_direction];
                    if(monster_can_enter_cell(labyrinth, monster, new_line, new_col)){
                        monster->line = new_line;
                        monster->col = new_col;
                    }
                    else{
                        break;
                    }
                    break;
                }
            }
            if(monster->line == selected_line && monster->col == selected_col){
                break;
            }
            continue;
        }

        for(int direction = 0 ; direction < 4 ; direction++){
            int new_line = monster->line + line_offsets[direction];
            int new_col = monster->col + col_offsets[direction];
            if(!monster_can_enter_cell(labyrinth, monster, new_line, new_col)){
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