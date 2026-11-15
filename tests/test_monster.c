#include <stdlib.h>
#include "minunit.h"
#include "test_helpers.h"
#include "generator.h"
#include "monster.h"

static Labyrinth* build_labyrinth_with_one_monster(const char** map, int height, int width,
                                                   int line, int col, char representation){
    Labyrinth* labyrinth = build_labyrinth_from_map(map, height, width);
    if(labyrinth == NULL){
        return NULL;
    }

    labyrinth->monsters = malloc(sizeof(Monster));
    if(labyrinth->monsters == NULL){
        free_labyrinth(labyrinth);
        return NULL;
    }
    labyrinth->monster_count = 1;

    int is_ghost = representation == GHOST_REPRESENTATION;
    labyrinth->monsters[0].line = line;
    labyrinth->monsters[0].col = col;
    labyrinth->monsters[0].representation = representation;
    labyrinth->monsters[0].penalty_points = is_ghost ? GHOST_PENALITY : OGRE_PENALITY;
    labyrinth->monsters[0].mobility = is_ghost ? move_ghost : move_ogre;
    labyrinth->monsters[0].patrol_line_step = 0;
    labyrinth->monsters[0].patrol_col_step = 0;
    return labyrinth;
}

static const char* test_add_monsters_too_small(void){
    Labyrinth* labyrinth = NULL;
    char name[] = "petit";
    mu_assert("génération impossible", generate_labyrinth(&labyrinth, name, EASY, 5, 5) == LABYRINTH_SUCCESS);

    LabyrinthStatus status = add_random_monsters(labyrinth);
    int count = labyrinth->monster_count;
    free_labyrinth(labyrinth);

    mu_assert("un petit labyrinthe ne doit pas avoir de monstres", status == LABYRINTH_NO_CHANGE && count == 0);
    return NULL;
}

static const char* test_add_monsters(void){
    Labyrinth* labyrinth = NULL;
    char name[] = "grand";
    mu_assert("génération impossible", generate_labyrinth(&labyrinth, name, EASY, 11, 25) == LABYRINTH_SUCCESS);

    mu_assert("l'ajout des monstres doit réussir", add_random_monsters(labyrinth) == LABYRINTH_SUCCESS);
    mu_assert("il doit y avoir des monstres", labyrinth->monster_count > 0 && labyrinth->monsters != NULL);

    int valid = 1;
    int ghosts = 0;
    int ogres = 0;
    for(int i = 0 ; i < labyrinth->monster_count ; i++){
        Monster* monster = &labyrinth->monsters[i];
        if(labyrinth->cells[monster->line][monster->col] != EMPTY || monster->mobility == NULL){
            valid = 0;
        }
        for(int j = i + 1 ; j < labyrinth->monster_count ; j++){
            if(monster->line == labyrinth->monsters[j].line && monster->col == labyrinth->monsters[j].col){
                valid = 0;
            }
        }
        if(monster->representation == GHOST_REPRESENTATION){
            ghosts++;
        }
        else if(monster->representation == OGRE_REPRESENTATION){
            ogres++;
        }
    }
    int count = labyrinth->monster_count;
    free_labyrinth(labyrinth);

    mu_assert("les monstres doivent être sur des cases vides, toutes différentes, et avoir un déplacement", valid);
    mu_assert("il doit y avoir des fantômes et des ogres", ghosts > 0 && ogres > 0 && ghosts + ogres == count);
    return NULL;
}

static const char* test_ghost_stays_in_range(void){
    const char* map[] = {
        "#########",
        "#.......#",
        "#.......#",
        "#.......#",
        "#.......#",
        "#.......#",
        "#########"
    };
    Labyrinth* labyrinth = build_labyrinth_with_one_monster(map, 7, 9, 3, 4, GHOST_REPRESENTATION);
    mu_assert("allocation impossible", labyrinth != NULL);

    Monster* ghost = &labyrinth->monsters[0];
    int valid = 1;
    for(int i = 0 ; i < 50 ; i++){
        int previous_line = ghost->line;
        int previous_col = ghost->col;
        move_ghost(labyrinth, ghost);

        int distance = abs(ghost->line - previous_line) + abs(ghost->col - previous_col);
        if(distance > GHOST_PENALITY || labyrinth->cells[ghost->line][ghost->col] == WALL){
            valid = 0;
        }
    }
    free_labyrinth(labyrinth);

    mu_assert("le fantôme doit rester dans sa portée et ne pas entrer dans un mur", valid);
    return NULL;
}

static const char* test_ghost_does_not_stack(void){
    const char* map[] = {
        "#####",
        "#...#",
        "#####"
    };
    Labyrinth* labyrinth = build_labyrinth_with_one_monster(map, 3, 5, 1, 1, GHOST_REPRESENTATION);
    mu_assert("allocation impossible", labyrinth != NULL);

    // Un deuxième monstre occupe une autre case du couloir
    labyrinth->monsters = realloc(labyrinth->monsters, 2 * sizeof(Monster));
    labyrinth->monsters[1] = labyrinth->monsters[0];
    labyrinth->monsters[1].line = 1;
    labyrinth->monsters[1].col = 3;
    labyrinth->monster_count = 2;

    int valid = 1;
    for(int i = 0 ; i < 30 ; i++){
        move_ghost(labyrinth, &labyrinth->monsters[0]);
        if(labyrinth->monsters[0].line == labyrinth->monsters[1].line &&
           labyrinth->monsters[0].col == labyrinth->monsters[1].col){
            valid = 0;
        }
    }
    free_labyrinth(labyrinth);

    mu_assert("deux monstres ne doivent pas se retrouver sur la même case", valid);
    return NULL;
}

static const char* test_ogre_goes_to_treasure(void){
    const char* map[] = {
        "###############",
        "#............$#",
        "###############"
    };
    Labyrinth* labyrinth = build_labyrinth_with_one_monster(map, 3, 15, 1, 1, OGRE_REPRESENTATION);
    mu_assert("allocation impossible", labyrinth != NULL);

    Monster* ogre = &labyrinth->monsters[0];
    for(int i = 0 ; i < 10 ; i++){
        move_ogre(labyrinth, ogre);
    }
    int distance_to_treasure = abs(ogre->col - 13);
    free_labyrinth(labyrinth);

    mu_assert("l'ogre doit s'être rapproché du trésor", distance_to_treasure <= OGRE_PATROL_RANGE);
    return NULL;
}

static const char* test_ogre_patrols_back_and_forth(void){
    const char* map[] = {
        "###############",
        "#............$#",
        "###############"
    };
    // L'ogre démarre à côté du trésor
    Labyrinth* labyrinth = build_labyrinth_with_one_monster(map, 3, 15, 1, 12, OGRE_REPRESENTATION);
    mu_assert("allocation impossible", labyrinth != NULL);

    Monster* ogre = &labyrinth->monsters[0];
    int visited[15] = {0};
    int valid = 1;
    for(int i = 0 ; i < 20 ; i++){
        move_ogre(labyrinth, ogre);
        visited[ogre->col] = 1;

        if(labyrinth->cells[ogre->line][ogre->col] == TREASURE ||
           labyrinth->cells[ogre->line][ogre->col] == WALL ||
           abs(ogre->col - 13) > OGRE_PATROL_RANGE){
            valid = 0;
        }
    }
    int distinct_cells = 0;
    for(int col = 0 ; col < 15 ; col++){
        distinct_cells += visited[col];
    }
    free_labyrinth(labyrinth);

    mu_assert("l'ogre ne doit pas entrer dans le trésor/un mur ni s'éloigner de sa zone de ronde", valid);
    mu_assert("l'ogre doit se déplacer entre plusieurs cases", distinct_cells >= 2);
    return NULL;
}

static const char* test_ogre_without_treasure_moves(void){
    const char* map[] = {
        "#######",
        "#.....#",
        "#.....#",
        "#######"
    };
    Labyrinth* labyrinth = build_labyrinth_with_one_monster(map, 4, 7, 1, 1, OGRE_REPRESENTATION);
    mu_assert("allocation impossible", labyrinth != NULL);

    Monster* ogre = &labyrinth->monsters[0];
    int moved = 0;
    int valid = 1;
    for(int i = 0 ; i < 10 ; i++){
        int previous_line = ogre->line;
        int previous_col = ogre->col;
        move_ogre(labyrinth, ogre);
        if(ogre->line != previous_line || ogre->col != previous_col){
            moved = 1;
        }
        if(labyrinth->cells[ogre->line][ogre->col] == WALL){
            valid = 0;
        }
    }
    free_labyrinth(labyrinth);

    mu_assert("l'ogre doit bouger même sans trésor", moved);
    mu_assert("l'ogre ne doit pas entrer dans un mur", valid);
    return NULL;
}

void run_monster_tests(void){
    mu_run_test(test_add_monsters_too_small);
    mu_run_test(test_add_monsters);
    mu_run_test(test_ghost_stays_in_range);
    mu_run_test(test_ghost_does_not_stack);
    mu_run_test(test_ogre_goes_to_treasure);
    mu_run_test(test_ogre_patrols_back_and_forth);
    mu_run_test(test_ogre_without_treasure_moves);
}
