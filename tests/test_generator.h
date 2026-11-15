#include <string.h>
#include "minunit.h"
#include "test_helpers.h"
#include "generator.h"

/**
 * Compte les cases accessibles depuis le joueur (en traversant tout sauf les murs).
 */
static int count_reachable_cells(Labyrinth* labyrinth, int start_line, int start_col){
    int height = labyrinth->height;
    int width = labyrinth->width;
    int* visited = calloc(height * width, sizeof(int));
    int* stack = malloc(height * width * sizeof(int));
    if(visited == NULL || stack == NULL){
        free(visited);
        free(stack);
        return -1;
    }

    int offsets_line[] = {-1, 1, 0, 0};
    int offsets_col[] = {0, 0, -1, 1};
    int stack_size = 0;
    int reachable = 0;

    stack[stack_size++] = start_line * width + start_col;
    visited[start_line * width + start_col] = 1;
    while(stack_size > 0){
        int current = stack[--stack_size];
        reachable++;
        for(int d = 0 ; d < 4 ; d++){
            int line = current / width + offsets_line[d];
            int col = current % width + offsets_col[d];
            if(line < 0 || line >= height || col < 0 || col >= width ||
               labyrinth->cells[line][col] == WALL || visited[line * width + col]){
                continue;
            }
            visited[line * width + col] = 1;
            stack[stack_size++] = line * width + col;
        }
    }

    free(visited);
    free(stack);
    return reachable;
}

MU_TEST(test_generate_easy){
    Labyrinth* labyrinth = NULL;
    char name[] = "gen_facile";
    LabyrinthStatus status = generate_labyrinth(&labyrinth, name, EASY, 11, 25);

    mu_assert(status == LABYRINTH_SUCCESS && labyrinth != NULL, "la génération doit réussir");
    mu_assert(labyrinth->height == 11 && labyrinth->width == 25, "mauvaises dimensions");
    mu_assert(count_cells(labyrinth, PLAYER) == 1, "il doit y avoir un seul joueur");
    mu_assert(count_cells(labyrinth, DOOR) == 1, "il doit y avoir une seule porte");
    mu_assert(count_cells(labyrinth, KEY) == 1, "il doit y avoir une seule clé");

    int border_ok = 1;
    for(int col = 0 ; col < 25 ; col++){
        if(labyrinth->cells[0][col] == EMPTY || labyrinth->cells[10][col] == EMPTY){
            border_ok = 0;
        }
    }
    free_labyrinth(labyrinth);
    mu_assert(border_ok, "les bords ne doivent pas être des cases vides");
}

MU_TEST(test_generate_hard){
    Labyrinth* labyrinth = NULL;
    char name[] = "gen_difficile";
    LabyrinthStatus status = generate_labyrinth(&labyrinth, name, HARD, 11, 25);

    mu_assert(status == LABYRINTH_SUCCESS && labyrinth != NULL, "la génération doit réussir");
    mu_assert(labyrinth->difficulty == HARD, "mauvaise difficulté");
    free_labyrinth(labyrinth);
}

MU_TEST(test_generate_default_size){
    Labyrinth* labyrinth = NULL;
    char name[] = "gen_defaut";
    LabyrinthStatus status = generate_labyrinth(&labyrinth, name, EASY, 0, 0);

    mu_assert(status == LABYRINTH_SUCCESS && labyrinth != NULL, "la génération doit réussir");
    mu_assert(labyrinth->height == DEFAULT_LABYRINTH_HEIGHT, "la hauteur par défaut doit être utilisée");
    mu_assert(labyrinth->width == DEFAULT_LABYRINTH_WIDTH, "la largeur par défaut doit être utilisée");
    free_labyrinth(labyrinth);
}

MU_TEST(test_generate_invalid_params){
    Labyrinth* labyrinth = NULL;
    char name[] = "gen_invalide";

    mu_assert(generate_labyrinth(&labyrinth, name, EASY, 10, 25) == LABYRINTH_CORRUPTED, "une hauteur paire doit être refusée");
    mu_assert(generate_labyrinth(&labyrinth, name, EASY, 11, 1) == LABYRINTH_CORRUPTED, "une largeur trop petite doit être refusée");
}

MU_TEST(test_generate_invalid_name){
    Labyrinth* labyrinth = NULL;
    char name[] = "nom invalide";

    mu_assert(generate_labyrinth(&labyrinth, name, EASY, 11, 25) != LABYRINTH_SUCCESS, "un nom invalide doit être refusé");
}

MU_TEST(test_generate_all_cells_reachable){
    Labyrinth* labyrinth = NULL;
    char name[] = "gen_accessible";
    mu_assert(generate_labyrinth(&labyrinth, name, EASY, 11, 25) == LABYRINTH_SUCCESS, "la génération doit réussir");

    int reachable = count_reachable_cells(labyrinth, 0, 1); // le joueur démarre en (0, 1)
    int expected = labyrinth->height * labyrinth->width - count_cells(labyrinth, WALL);
    free_labyrinth(labyrinth);

    mu_assert(reachable == expected, "toutes les cases non-murs doivent être accessibles depuis le joueur");
}

MU_TEST_SUITE(generator_suite){
    MU_RUN_TEST(test_generate_easy);
    MU_RUN_TEST(test_generate_hard);
    MU_RUN_TEST(test_generate_default_size);
    MU_RUN_TEST(test_generate_invalid_params);
    MU_RUN_TEST(test_generate_invalid_name);
    MU_RUN_TEST(test_generate_all_cells_reachable);
}
