#include <string.h>
#include "minunit.h"
#include "test_helpers.h"

static const char* test_allocate_valid(void){
    char name[MAX_LABYRINTH_NAME_LENGTH + 1] = "mon_labyrinthe";
    Labyrinth* labyrinth = allocate_labyrinth(name, HARD, 5, 7);

    mu_assert("allocate_labyrinth doit retourner un labyrinthe", labyrinth != NULL);
    mu_assert("mauvaise hauteur", labyrinth->height == 5);
    mu_assert("mauvaise largeur", labyrinth->width == 7);
    mu_assert("mauvaise difficulté", labyrinth->difficulty == HARD);
    mu_assert("mauvais nom", strcmp(labyrinth->name, "mon_labyrinthe") == 0);
    mu_assert("la porte doit être verrouillée", labyrinth->door_unlocked == 0);
    mu_assert("aucun monstre au départ", labyrinth->monster_count == 0 && labyrinth->monsters == NULL);

    free_labyrinth(labyrinth);
    return NULL;
}

static const char* test_allocate_invalid_name(void){
    char name[MAX_LABYRINTH_NAME_LENGTH + 1] = "nom invalide";
    mu_assert("un nom invalide doit donner NULL", allocate_labyrinth(name, EASY, 5, 5) == NULL);
    return NULL;
}

static const char* test_free_null(void){
    free_labyrinth(NULL); // ne doit pas planter
    return NULL;
}

static const char* test_valid_names(void){
    char valid[] = "Labyrinthe_1-ok";
    char with_space[] = "a b";
    char accent[] = "été";
    char empty[] = "";
    char too_long[MAX_LABYRINTH_NAME_LENGTH + 2];
    memset(too_long, 'a', sizeof(too_long) - 1);
    too_long[sizeof(too_long) - 1] = '\0';

    mu_assert("nom valide refusé", is_valid_labyrinth_name(valid));
    mu_assert("NULL accepté", !is_valid_labyrinth_name(NULL));
    mu_assert("nom vide accepté", !is_valid_labyrinth_name(empty));
    mu_assert("espace accepté", !is_valid_labyrinth_name(with_space));
    mu_assert("accents acceptés", !is_valid_labyrinth_name(accent));
    mu_assert("nom trop long accepté", !is_valid_labyrinth_name(too_long));
    return NULL;
}

static const char* test_valid_params(void){
    mu_assert("11x25 facile doit être valide", is_valid_labyrinth_params(EASY, 11, 25));
    mu_assert("11x25 difficile doit être valide", is_valid_labyrinth_params(HARD, 11, 25));
    mu_assert("dimension paire acceptée", !is_valid_labyrinth_params(EASY, 10, 25));
    mu_assert("dimension trop petite acceptée", !is_valid_labyrinth_params(EASY, 1, 25));
    mu_assert("difficulté inconnue acceptée", !is_valid_labyrinth_params((Difficulty)5, 11, 25));
    return NULL;
}

static const char* test_cell_representation_roundtrip(void){
    Cell cells[] = {EMPTY, WALL, DOOR, KEY, TREASURE, TRAP, PLAYER};
    for(size_t i = 0 ; i < sizeof(cells) / sizeof(cells[0]) ; i++){
        mu_assert("get_cell(get_cell_representation(x)) doit retourner x",
                  get_cell(get_cell_representation(cells[i])) == cells[i]);
    }
    return NULL;
}

static const char* test_cell_unknown(void){
    mu_assert("un caractère inconnu doit donner UNKNOWN", get_cell('?') == UNKNOWN);
    mu_assert("UNKNOWN doit donner 'E'", get_cell_representation(UNKNOWN) == 'E');
    return NULL;
}

static const char* test_add_random_objects(void){
    char name[MAX_LABYRINTH_NAME_LENGTH + 1] = "objets";
    Labyrinth* labyrinth = allocate_labyrinth(name, EASY, 11, 25);
    mu_assert("allocation impossible", labyrinth != NULL);
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

    mu_assert("il doit y avoir exactement une clé", keys == 1);
    mu_assert("il doit y avoir des pièges", traps > 0);
    mu_assert("il doit y avoir des trésors", treasures > 0);
    return NULL;
}

static const char* test_break_random_walls(void){
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
    mu_assert("allocation impossible", labyrinth != NULL);

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

    mu_assert("des murs doivent être supprimés", walls_after < walls_before);
    mu_assert("les murs extérieurs ne doivent pas être supprimés", border_intact);
    return NULL;
}

void run_labyrinth_tests(void){
    mu_run_test(test_allocate_valid);
    mu_run_test(test_allocate_invalid_name);
    mu_run_test(test_free_null);
    mu_run_test(test_valid_names);
    mu_run_test(test_valid_params);
    mu_run_test(test_cell_representation_roundtrip);
    mu_run_test(test_cell_unknown);
    mu_run_test(test_add_random_objects);
    mu_run_test(test_break_random_walls);
}
