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

static const char* test_generate_easy(void){
    Labyrinth* labyrinth = NULL;
    char name[] = "gen_facile";
    LabyrinthStatus status = generate_labyrinth(&labyrinth, name, EASY, 11, 25);

    mu_assert("la génération doit réussir", status == LABYRINTH_SUCCESS && labyrinth != NULL);
    mu_assert("mauvaises dimensions", labyrinth->height == 11 && labyrinth->width == 25);
    mu_assert("il doit y avoir un seul joueur", count_cells(labyrinth, PLAYER) == 1);
    mu_assert("il doit y avoir une seule porte", count_cells(labyrinth, DOOR) == 1);
    mu_assert("il doit y avoir une seule clé", count_cells(labyrinth, KEY) == 1);

    int border_ok = 1;
    for(int col = 0 ; col < 25 ; col++){
        if(labyrinth->cells[0][col] == EMPTY || labyrinth->cells[10][col] == EMPTY){
            border_ok = 0;
        }
    }
    free_labyrinth(labyrinth);
    mu_assert("les bords ne doivent pas être des cases vides", border_ok);
    return NULL;
}

static const char* test_generate_hard(void){
    Labyrinth* labyrinth = NULL;
    char name[] = "gen_difficile";
    LabyrinthStatus status = generate_labyrinth(&labyrinth, name, HARD, 11, 25);

    mu_assert("la génération doit réussir", status == LABYRINTH_SUCCESS && labyrinth != NULL);
    mu_assert("mauvaise difficulté", labyrinth->difficulty == HARD);
    free_labyrinth(labyrinth);
    return NULL;
}

static const char* test_generate_default_size(void){
    Labyrinth* labyrinth = NULL;
    char name[] = "gen_defaut";
    LabyrinthStatus status = generate_labyrinth(&labyrinth, name, EASY, 0, 0);

    mu_assert("la génération doit réussir", status == LABYRINTH_SUCCESS && labyrinth != NULL);
    mu_assert("la hauteur par défaut doit être utilisée", labyrinth->height == DEFAULT_LABYRINTH_HEIGHT);
    mu_assert("la largeur par défaut doit être utilisée", labyrinth->width == DEFAULT_LABYRINTH_WIDTH);
    free_labyrinth(labyrinth);
    return NULL;
}

static const char* test_generate_invalid_params(void){
    Labyrinth* labyrinth = NULL;
    char name[] = "gen_invalide";

    mu_assert("une hauteur paire doit être refusée",
              generate_labyrinth(&labyrinth, name, EASY, 10, 25) == LABYRINTH_CORRUPTED);
    mu_assert("une largeur trop petite doit être refusée",
              generate_labyrinth(&labyrinth, name, EASY, 11, 1) == LABYRINTH_CORRUPTED);
    return NULL;
}

static const char* test_generate_invalid_name(void){
    Labyrinth* labyrinth = NULL;
    char name[] = "nom invalide";

    mu_assert("un nom invalide doit être refusé",
              generate_labyrinth(&labyrinth, name, EASY, 11, 25) != LABYRINTH_SUCCESS);
    return NULL;
}

static const char* test_generate_all_cells_reachable(void){
    Labyrinth* labyrinth = NULL;
    char name[] = "gen_accessible";
    mu_assert("la génération doit réussir",
              generate_labyrinth(&labyrinth, name, EASY, 11, 25) == LABYRINTH_SUCCESS);

    int reachable = count_reachable_cells(labyrinth, 0, 1); // le joueur démarre en (0, 1)
    int expected = labyrinth->height * labyrinth->width - count_cells(labyrinth, WALL);
    free_labyrinth(labyrinth);

    mu_assert("toutes les cases non-murs doivent être accessibles depuis le joueur", reachable == expected);
    return NULL;
}

void run_generator_tests(void){
    mu_run_test(test_generate_easy);
    mu_run_test(test_generate_hard);
    mu_run_test(test_generate_default_size);
    mu_run_test(test_generate_invalid_params);
    mu_run_test(test_generate_invalid_name);
    mu_run_test(test_generate_all_cells_reachable);
}
