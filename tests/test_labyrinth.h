#include <string.h>
#include "minunit.h"
#include "test_helpers.h"

MU_TEST(test_allocate_valid){
    char name[MAX_LABYRINTH_NAME_LENGTH + 1] = "mon_labyrinthe";
    Labyrinth* labyrinth = allocate_labyrinth(name, HARD, 5, 7);

    mu_assert(labyrinth != NULL, "allocate_labyrinth doit retourner un labyrinthe");
    mu_assert(labyrinth->height == 5, "mauvaise hauteur");
    mu_assert(labyrinth->width == 7, "mauvaise largeur");
    mu_assert(labyrinth->difficulty == HARD, "mauvaise difficulté");
    mu_assert(strcmp(labyrinth->name, "mon_labyrinthe") == 0, "mauvais nom");
    mu_assert(labyrinth->door_unlocked == 0, "la porte doit être verrouillée");
    mu_assert(labyrinth->monster_count == 0 && labyrinth->monsters == NULL, "aucun monstre au départ");

    free_labyrinth(labyrinth);
}

MU_TEST(test_allocate_invalid_name){
    char name[MAX_LABYRINTH_NAME_LENGTH + 1] = "nom invalide";
    mu_assert(allocate_labyrinth(name, EASY, 5, 5) == NULL, "un nom invalide doit donner NULL");
}

MU_TEST(test_free_null){
    free_labyrinth(NULL); // ne doit pas planter
    mu_check(1);
}

MU_TEST(test_valid_names){
    char valid[] = "Labyrinthe_1-ok";
    char with_space[] = "a b";
    char accent[] = "été";
    char empty[] = "";
    char too_long[MAX_LABYRINTH_NAME_LENGTH + 2];
    memset(too_long, 'a', sizeof(too_long) - 1);
    too_long[sizeof(too_long) - 1] = '\0';

    mu_assert(is_valid_labyrinth_name(valid), "nom valide refusé");
    mu_assert(!is_valid_labyrinth_name(NULL), "NULL accepté");
    mu_assert(!is_valid_labyrinth_name(empty), "nom vide accepté");
    mu_assert(!is_valid_labyrinth_name(with_space), "espace accepté");
    mu_assert(!is_valid_labyrinth_name(accent), "accents acceptés");
    mu_assert(!is_valid_labyrinth_name(too_long), "nom trop long accepté");
}

MU_TEST(test_valid_params){
    mu_assert(is_valid_labyrinth_params(EASY, 11, 25), "11x25 facile doit être valide");
    mu_assert(is_valid_labyrinth_params(HARD, 11, 25), "11x25 difficile doit être valide");
    mu_assert(!is_valid_labyrinth_params(EASY, 10, 25), "dimension paire acceptée");
    mu_assert(!is_valid_labyrinth_params(EASY, 1, 25), "dimension trop petite acceptée");
    mu_assert(!is_valid_labyrinth_params((Difficulty)5, 11, 25), "difficulté inconnue acceptée");
}

MU_TEST(test_cell_representation_roundtrip){
    Cell cells[] = {EMPTY, WALL, DOOR, KEY, TREASURE, TRAP, PLAYER};
    for(size_t i = 0 ; i < sizeof(cells) / sizeof(cells[0]) ; i++){
        mu_assert(get_cell(get_cell_representation(cells[i])) == cells[i], "get_cell(get_cell_representation(x)) doit retourner x");
    }
}

MU_TEST(test_cell_unknown){
    mu_assert(get_cell('?') == UNKNOWN, "un caractère inconnu doit donner UNKNOWN");
    mu_assert(get_cell_representation(UNKNOWN) == 'E', "UNKNOWN doit donner 'E'");
}

MU_TEST(test_add_random_objects){
    char name[MAX_LABYRINTH_NAME_LENGTH + 1] = "objets";
    Labyrinth* labyrinth = allocate_labyrinth(name, EASY, 11, 25);
    mu_assert(labyrinth != NULL, "allocation impossible");
    for(int line = 0 ; line < 11 ; line++){
        for(int col = 0 ; col < 25 ; col++){
            labyrinth->cells[line][col] = EMPTY;
        }
    }

    add_random_objects(labyrinth);

    int keys = count_cells(labyrinth, KEY);
    int traps = count_cells(labyrinth, TRAP);
    int treasures = count_cells(labyrinth, TREASURE);
    free_labyrinth(labyrinth);

    mu_assert(keys == 1, "il doit y avoir exactement une clé");
    mu_assert(traps > 0, "il doit y avoir des pièges");
    mu_assert(treasures > 0, "il doit y avoir des trésors");
}

MU_TEST(test_break_random_walls){
    const char* map[] = {
        "###########",
        "#.........#",
        "###########",
        "#.........#",
        "###########",
        "#.........#",
        "###########",
        "#.........#",
        "###########",
        "#.........#",
        "###########"
    };
    Labyrinth* labyrinth = build_labyrinth_from_map(map, 11, 11);
    mu_assert(labyrinth != NULL, "allocation impossible");

    int walls_before = count_cells(labyrinth, WALL);
    break_random_walls(labyrinth);
    int walls_after = count_cells(labyrinth, WALL);

    int border_intact = 1;
    for(int i = 0 ; i < 11 ; i++){
        if(labyrinth->cells[0][i] != WALL || labyrinth->cells[10][i] != WALL ||
           labyrinth->cells[i][0] != WALL || labyrinth->cells[i][10] != WALL){
            border_intact = 0;
        }
    }
    free_labyrinth(labyrinth);

    mu_assert(walls_after < walls_before, "des murs doivent être supprimés");
    mu_assert(border_intact, "les murs extérieurs ne doivent pas être supprimés");
}

MU_TEST_SUITE(labyrinth_suite){
    MU_RUN_TEST(test_allocate_valid);
    MU_RUN_TEST(test_allocate_invalid_name);
    MU_RUN_TEST(test_free_null);
    MU_RUN_TEST(test_valid_names);
    MU_RUN_TEST(test_valid_params);
    MU_RUN_TEST(test_cell_representation_roundtrip);
    MU_RUN_TEST(test_cell_unknown);
    MU_RUN_TEST(test_add_random_objects);
    MU_RUN_TEST(test_break_random_walls);
}
