#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "minunit.h"
#include "test_helpers.h"
#include "io.h"
#include "generator.h"

#define TEST_LABYRINTH_NAME "minunit_test_labyrinthe"
#define TEST_SCORES_NAME "minunit_test_scores"

/**
 * Supprime le fichier de sauvegarde (type 1 : labyrinthe, 2 : scores) du labyrinthe de test.
 */
static void remove_save_file(char* name, int save_type){
    char* path = build_save_file_path(name, save_type);
    if(path != NULL){
        remove(path);
        free(path);
    }
}

MU_TEST(test_build_save_file_path){
    char name[] = "maze";
    char* labyrinth_path = build_save_file_path(name, 1);
    char* scores_path = build_save_file_path(name, 2);
    char* invalid_path = build_save_file_path(name, 3);

    int labyrinth_ok = labyrinth_path != NULL && strcmp(labyrinth_path, LABYRINTHS_SAVE_PATH "maze.cfg") == 0;
    int scores_ok = scores_path != NULL && strcmp(scores_path, SCORES_SAVE_PATH "maze.score") == 0;
    int invalid_ok = invalid_path == NULL;

    free(labyrinth_path);
    free(scores_path);
    free(invalid_path);

    mu_assert(labyrinth_ok, "mauvais chemin pour un labyrinthe");
    mu_assert(scores_ok, "mauvais chemin pour des scores");
    mu_assert(invalid_ok, "un type inconnu doit donner NULL");
}

MU_TEST(test_is_valid_player_name){
    char too_long[MAX_PLAYER_NAME_LENGTH + 2];
    memset(too_long, 'a', sizeof(too_long) - 1);
    too_long[sizeof(too_long) - 1] = '\0';

    mu_assert(is_valid_player_name("alice"), "un nom simple doit être valide");
    mu_assert(!is_valid_player_name(""), "un nom vide doit être refusé");
    mu_assert(!is_valid_player_name("alice bob"), "un nom avec espace doit être refusé");
    mu_assert(!is_valid_player_name(too_long), "un nom trop long doit être refusé");
}

MU_TEST(test_save_and_load_labyrinth){
    char name[] = TEST_LABYRINTH_NAME;
    remove_save_file(name, 1);

    Labyrinth* original = NULL;
    mu_assert(generate_labyrinth(&original, name, HARD, 7, 9) == LABYRINTH_SUCCESS, "génération impossible");

    LabyrinthStatus save_status = save_labyrinth(name, original);
    LabyrinthStatus second_save_status = save_labyrinth(name, original);

    Labyrinth* loaded = NULL;
    LabyrinthStatus load_status = load_labyrinth(name, &loaded);
    remove_save_file(name, 1);

    int same = load_status == LABYRINTH_SUCCESS && loaded != NULL &&
               loaded->height == original->height &&
               loaded->width == original->width &&
               loaded->difficulty == original->difficulty &&
               strcmp(loaded->name, original->name) == 0;
    if(same){
        for(int line = 0 ; line < original->height ; line++){
            for(int col = 0 ; col < original->width ; col++){
                if(original->cells[line][col] != loaded->cells[line][col]){
                    same = 0;
                }
            }
        }
    }

    free_labyrinth(original);
    free_labyrinth(loaded);

    mu_assert(save_status == LABYRINTH_SUCCESS, "la sauvegarde doit réussir");
    mu_assert(second_save_status == LABYRINTH_ALREADY_EXISTS, "sauvegarder deux fois le même nom doit échouer");
    mu_assert(same, "le labyrinthe chargé doit être identique à l'original");
}

MU_TEST(test_save_labyrinth_invalid){
    Labyrinth* labyrinth = NULL;
    char name[] = "invalide_test";
    mu_assert(generate_labyrinth(&labyrinth, name, EASY, 5, 5) == LABYRINTH_SUCCESS, "génération impossible");

    char invalid_name[] = "nom invalide";
    LabyrinthStatus invalid_name_status = save_labyrinth(invalid_name, labyrinth);
    LabyrinthStatus null_status = save_labyrinth(name, NULL);
    free_labyrinth(labyrinth);

    mu_assert(invalid_name_status == LABYRINTH_INVALID_NAME, "un nom invalide doit être refusé");
    mu_assert(null_status == LABYRINTH_CORRUPTED, "un labyrinthe NULL doit être refusé");
}

MU_TEST(test_load_labyrinth_not_found){
    Labyrinth* labyrinth = (Labyrinth*)1;
    char name[] = "labyrinthe_qui_nexiste_pas";
    LabyrinthStatus status = load_labyrinth(name, &labyrinth);

    mu_assert(status == LABYRINTH_NOT_FOUND, "un labyrinthe inexistant doit donner NOT_FOUND");
    mu_assert(labyrinth == NULL, "le pointeur doit être NULL après un échec");
}

MU_TEST(test_save_and_load_scores){
    char name[] = TEST_SCORES_NAME;
    remove_save_file(name, 2);

    LabyrinthStatus status_alice = save_score(name, "alice", 100);
    LabyrinthStatus status_bob = save_score(name, "bob", 300);
    LabyrinthStatus status_carol = save_score(name, "carol", 200);

    long int scores[MAX_SCORES];
    char player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1];
    LabyrinthStatus load_status = load_scores(name, scores, player_names);
    remove_save_file(name, 2);

    mu_assert(status_alice == LABYRINTH_SUCCESS && status_bob == LABYRINTH_SUCCESS && status_carol == LABYRINTH_SUCCESS, "les sauvegardes de scores doivent réussir");
    mu_assert(load_status == LABYRINTH_SUCCESS, "le chargement des scores doit réussir");
    mu_assert(scores[0] == 300 && scores[1] == 200 && scores[2] == 100, "les scores doivent être classés du plus grand au plus petit");
    mu_assert(strcmp(player_names[0], "bob") == 0 && strcmp(player_names[1], "carol") == 0 && strcmp(player_names[2], "alice") == 0, "les noms doivent suivre le classement");
    mu_assert(scores[3] == -1 && player_names[3][0] == '\0', "les cases inutilisées doivent valoir -1");
}

MU_TEST(test_save_score_invalid){
    char name[] = TEST_SCORES_NAME;
    remove_save_file(name, 2);

    mu_assert(save_score(name, "alice", 0) == LABYRINTH_INVALID_FORMAT, "un score nul doit être refusé");
    mu_assert(save_score(name, "alice bob", 10) == LABYRINTH_INVALID_NAME, "un nom de joueur invalide doit être refusé");
    mu_assert(save_score("nom invalide", "alice", 10) == LABYRINTH_INVALID_NAME, "un nom de labyrinthe invalide doit être refusé");
}

MU_TEST(test_save_score_not_in_top){
    char name[] = TEST_SCORES_NAME;
    remove_save_file(name, 2);

    for(int i = 0 ; i < MAX_SCORES ; i++){
        save_score(name, "joueur", 1000 + i);
    }
    LabyrinthStatus status = save_score(name, "perdant", 5);
    remove_save_file(name, 2);

    mu_assert(status == LABYRINTH_NO_CHANGE, "un score trop faible ne doit pas être enregistré");
}

MU_TEST(test_load_scores_not_found){
    long int scores[MAX_SCORES];
    char player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1];
    char name[] = "scores_qui_nexistent_pas";

    mu_assert(load_scores(name, scores, player_names) == LABYRINTH_NOT_FOUND, "des scores inexistants doivent donner NOT_FOUND");
}

MU_TEST_SUITE(io_suite){
    MU_RUN_TEST(test_build_save_file_path);
    MU_RUN_TEST(test_is_valid_player_name);
    MU_RUN_TEST(test_save_and_load_labyrinth);
    MU_RUN_TEST(test_save_labyrinth_invalid);
    MU_RUN_TEST(test_load_labyrinth_not_found);
    MU_RUN_TEST(test_save_and_load_scores);
    MU_RUN_TEST(test_save_score_invalid);
    MU_RUN_TEST(test_save_score_not_in_top);
    MU_RUN_TEST(test_load_scores_not_found);
}
