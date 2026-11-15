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
            labyrinth->monsters[i].patrol_line_step = 0;
            labyrinth->monsters[i].patrol_col_step = 0;

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

static const int OGRE_LINE_OFFSETS[] = {-1, 1, 0, 0};
static const int OGRE_COL_OFFSETS[] = {0, 0, -1, 1};

/**
 * Cherche le trésor le plus proche (distance de Manhattan) d'une position.
 * Retourne 1 si un trésor existe (et remplit line/col), 0 sinon.
 */
static int find_nearest_treasure(Labyrinth* labyrinth, int from_line, int from_col, int* line, int* col){
    int best_distance = -1;

    for(int l = 0 ; l < labyrinth->height ; l++){
        for(int c = 0 ; c < labyrinth->width ; c++){
            if(labyrinth->cells[l][c] != TREASURE){
                continue;
            }
            int distance = abs(l - from_line) + abs(c - from_col);
            if(best_distance == -1 || distance < best_distance){
                best_distance = distance;
                *line = l;
                *col = c;
            }
        }
    }

    return best_distance != -1;
}

static int is_near_treasure(int line, int col, int treasure_line, int treasure_col){
    return abs(line - treasure_line) <= OGRE_PATROL_RANGE &&
           abs(col - treasure_col) <= OGRE_PATROL_RANGE;
}

/**
 * Un pas de ronde : l'ogre avance dans sa direction courante et fait demi-tour
 * quand il est bloqué ou qu'il s'éloignerait trop du trésor.
 */
static void ogre_patrol_step(Labyrinth* labyrinth, Monster* monster, int treasure_line, int treasure_col){
    for(int attempt = 0 ; attempt < 2 && (monster->patrol_line_step != 0 || monster->patrol_col_step != 0) ; attempt++){
        int new_line = monster->line + monster->patrol_line_step;
        int new_col = monster->col + monster->patrol_col_step;
        if(is_near_treasure(new_line, new_col, treasure_line, treasure_col) &&
           monster_can_enter_cell(labyrinth, monster, new_line, new_col)){
            monster->line = new_line;
            monster->col = new_col;
            return;
        }
        // Bloqué : demi-tour
        monster->patrol_line_step = -monster->patrol_line_step;
        monster->patrol_col_step = -monster->patrol_col_step;
    }

    // Pas de direction (début de ronde) ou coincé dans les deux sens : on choisit une direction libre au hasard
    int candidate_count = 0;
    for(int direction = 0 ; direction < 4 ; direction++){
        int new_line = monster->line + OGRE_LINE_OFFSETS[direction];
        int new_col = monster->col + OGRE_COL_OFFSETS[direction];
        if(is_near_treasure(new_line, new_col, treasure_line, treasure_col) &&
           monster_can_enter_cell(labyrinth, monster, new_line, new_col)){
            candidate_count++;
            if(rand() % candidate_count == 0){
                monster->patrol_line_step = OGRE_LINE_OFFSETS[direction];
                monster->patrol_col_step = OGRE_COL_OFFSETS[direction];
            }
        }
    }
    if(candidate_count > 0){
        monster->line += monster->patrol_line_step;
        monster->col += monster->patrol_col_step;
    }
}

/**
 * Un pas vers le trésor (ou au hasard s'il n'y a pas de trésor ou si l'ogre est bloqué
 * contre un mur). Retourne 0 si l'ogre n'a pas pu bouger.
 */
static int ogre_approach_step(Labyrinth* labyrinth, Monster* monster, int has_treasure, int treasure_line, int treasure_col){
    int selected_line = monster->line;
    int selected_col = monster->col;
    int selected_distance = has_treasure ? abs(treasure_line - monster->line) + abs(treasure_col - monster->col) : 0;
    int candidate_count = 0;
    int random_candidate_count = 0;
    int random_line = monster->line;
    int random_col = monster->col;

    for(int direction = 0 ; direction < 4 ; direction++){
        int new_line = monster->line + OGRE_LINE_OFFSETS[direction];
        int new_col = monster->col + OGRE_COL_OFFSETS[direction];
        if(!monster_can_enter_cell(labyrinth, monster, new_line, new_col)){
            continue;
        }

        random_candidate_count++;
        if(rand() % random_candidate_count == 0){
            random_line = new_line;
            random_col = new_col;
        }

        if(has_treasure){
            int distance = abs(treasure_line - new_line) + abs(treasure_col - new_col);
            if(distance < selected_distance){
                selected_distance = distance;
                selected_line = new_line;
                selected_col = new_col;
                candidate_count = 1;
            }
            else if(distance == selected_distance && selected_distance < abs(treasure_line - monster->line) + abs(treasure_col - monster->col) &&
                    rand() % ++candidate_count == 0){
                selected_line = new_line;
                selected_col = new_col;
            }
        }
    }

    if(selected_line == monster->line && selected_col == monster->col){
        // Aucun pas ne rapproche du trésor : mouvement aléatoire pour se débloquer
        selected_line = random_line;
        selected_col = random_col;
    }

    if(selected_line == monster->line && selected_col == monster->col){
        return 0;
    }
    monster->line = selected_line;
    monster->col = selected_col;
    return 1;
}

void move_ogre(Labyrinth* labyrinth, Monster* monster){
    for(int step = 0 ; step < monster->penalty_points ; step++){
        int treasure_line = -1;
        int treasure_col = -1;
        int has_treasure = find_nearest_treasure(labyrinth, monster->line, monster->col, &treasure_line, &treasure_col);

        if(has_treasure && is_near_treasure(monster->line, monster->col, treasure_line, treasure_col)){
            ogre_patrol_step(labyrinth, monster, treasure_line, treasure_col);
        }
        else if(!ogre_approach_step(labyrinth, monster, has_treasure, treasure_line, treasure_col)){
            break;
        }
    }
}
