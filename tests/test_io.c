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

static const char* test_build_save_file_path(void){
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

    mu_assert("mauvais chemin pour un labyrinthe", labyrinth_ok);
    mu_assert("mauvais chemin pour des scores", scores_ok);
    mu_assert("un type inconnu doit donner NULL", invalid_ok);
    return NULL;
}

static const char* test_is_valid_player_name(void){
    char too_long[MAX_PLAYER_NAME_LENGTH + 2];
    memset(too_long, 'a', sizeof(too_long) - 1);
    too_long[sizeof(too_long) - 1] = '\0';

    mu_assert("un nom simple doit être valide", is_valid_player_name("alice"));
    mu_assert("un nom vide doit être refusé", !is_valid_player_name(""));
    mu_assert("un nom avec espace doit être refusé", !is_valid_player_name("alice bob"));
    mu_assert("un nom trop long doit être refusé", !is_valid_player_name(too_long));
    return NULL;
}

static const char* test_save_and_load_labyrinth(void){
    char name[] = TEST_LABYRINTH_NAME;
    remove_save_file(name, 1);

    Labyrinth* original = NULL;
    mu_assert("génération impossible", generate_labyrinth(&original, name, HARD, 7, 9) == LABYRINTH_SUCCESS);

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

    mu_assert("la sauvegarde doit réussir", save_status == LABYRINTH_SUCCESS);
    mu_assert("sauvegarder deux fois le même nom doit échouer", second_save_status == LABYRINTH_ALREADY_EXISTS);
    mu_assert("le labyrinthe chargé doit être identique à l'original", same);
    return NULL;
}

static const char* test_save_labyrinth_invalid(void){
    Labyrinth* labyrinth = NULL;
    char name[] = "invalide_test";
    mu_assert("génération impossible", generate_labyrinth(&labyrinth, name, EASY, 5, 5) == LABYRINTH_SUCCESS);

    char invalid_name[] = "nom invalide";
    LabyrinthStatus invalid_name_status = save_labyrinth(invalid_name, labyrinth);
    LabyrinthStatus null_status = save_labyrinth(name, NULL);
    free_labyrinth(labyrinth);

    mu_assert("un nom invalide doit être refusé", invalid_name_status == LABYRINTH_INVALID_NAME);
    mu_assert("un labyrinthe NULL doit être refusé", null_status == LABYRINTH_CORRUPTED);
    return NULL;
}

static const char* test_load_labyrinth_not_found(void){
    Labyrinth* labyrinth = (Labyrinth*)1;
    char name[] = "labyrinthe_qui_nexiste_pas";
    LabyrinthStatus status = load_labyrinth(name, &labyrinth);

    mu_assert("un labyrinthe inexistant doit donner NOT_FOUND", status == LABYRINTH_NOT_FOUND);
    mu_assert("le pointeur doit être NULL après un échec", labyrinth == NULL);
    return NULL;
}

static const char* test_save_and_load_scores(void){
    char name[] = TEST_SCORES_NAME;
    remove_save_file(name, 2);

    LabyrinthStatus status_alice = save_score(name, "alice", 100);
    LabyrinthStatus status_bob = save_score(name, "bob", 300);
    LabyrinthStatus status_carol = save_score(name, "carol", 200);

    long int scores[MAX_SCORES];
    char player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1];
    LabyrinthStatus load_status = load_scores(name, scores, player_names);
    remove_save_file(name, 2);

    mu_assert("les sauvegardes de scores doivent réussir",
              status_alice == LABYRINTH_SUCCESS && status_bob == LABYRINTH_SUCCESS && status_carol == LABYRINTH_SUCCESS);
    mu_assert("le chargement des scores doit réussir", load_status == LABYRINTH_SUCCESS);
    mu_assert("les scores doivent être classés du plus grand au plus petit",
              scores[0] == 300 && scores[1] == 200 && scores[2] == 100);
    mu_assert("les noms doivent suivre le classement",
              strcmp(player_names[0], "bob") == 0 && strcmp(player_names[1], "carol") == 0 && strcmp(player_names[2], "alice") == 0);
    mu_assert("les cases inutilisées doivent valoir -1", scores[3] == -1 && player_names[3][0] == '\0');
    return NULL;
}

static const char* test_save_score_invalid(void){
    char name[] = TEST_SCORES_NAME;
    remove_save_file(name, 2);

    mu_assert("un score nul doit être refusé", save_score(name, "alice", 0) == LABYRINTH_INVALID_FORMAT);
    mu_assert("un nom de joueur invalide doit être refusé", save_score(name, "alice bob", 10) == LABYRINTH_INVALID_NAME);
    mu_assert("un nom de labyrinthe invalide doit être refusé", save_score("nom invalide", "alice", 10) == LABYRINTH_INVALID_NAME);
    return NULL;
}

static const char* test_save_score_not_in_top(void){
    char name[] = TEST_SCORES_NAME;
    remove_save_file(name, 2);

    for(int i = 0 ; i < MAX_SCORES ; i++){
        save_score(name, "joueur", 1000 + i);
    }
    LabyrinthStatus status = save_score(name, "perdant", 5);
    remove_save_file(name, 2);

    mu_assert("un score trop faible ne doit pas être enregistré", status == LABYRINTH_NO_CHANGE);
    return NULL;
}

static const char* test_load_scores_not_found(void){
    long int scores[MAX_SCORES];
    char player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1];
    char name[] = "scores_qui_nexistent_pas";

    mu_assert("des scores inexistants doivent donner NOT_FOUND",
              load_scores(name, scores, player_names) == LABYRINTH_NOT_FOUND);
    return NULL;
}

void run_io_tests(void){
    mu_run_test(test_build_save_file_path);
    mu_run_test(test_is_valid_player_name);
    mu_run_test(test_save_and_load_labyrinth);
    mu_run_test(test_save_labyrinth_invalid);
    mu_run_test(test_load_labyrinth_not_found);
    mu_run_test(test_save_and_load_scores);
    mu_run_test(test_save_score_invalid);
    mu_run_test(test_save_score_not_in_top);
    mu_run_test(test_load_scores_not_found);
}
