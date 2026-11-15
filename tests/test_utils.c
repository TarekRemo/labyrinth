#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "minunit.h"
#include "utils.h"

#define TEST_FOLDER "build/test_utils_folder"
#define TEST_FILE TEST_FOLDER "/fichier.txt"

/**
 * Appelle read_int sur le texte donné et retourne son résultat.
 */
static int read_int_from_text(const char* text, int* value){
    FILE* file = tmpfile();
    if(file == NULL){
        return -1;
    }
    fputs(text, file);
    rewind(file);
    int result = read_int(file, value);
    fclose(file);
    return result;
}

static const char* test_read_int_valid(void){
    int value = 0;
    mu_assert("un entier valide doit être lu", read_int_from_text("42\n", &value) == 1 && value == 42);
    mu_assert("un entier négatif doit être lu", read_int_from_text("-7\n", &value) == 1 && value == -7);
    return NULL;
}

static const char* test_read_int_invalid(void){
    int value = 0;
    mu_assert("du texte doit être refusé", read_int_from_text("abc\n", &value) == 0);
    mu_assert("un nombre suivi de texte doit être refusé", read_int_from_text("12abc\n", &value) == 0);
    mu_assert("une ligne vide doit être refusée", read_int_from_text("\n", &value) == 0);
    mu_assert("un nombre trop grand doit être refusé", read_int_from_text("99999999999999\n", &value) == 0);
    mu_assert("NULL doit être refusé", read_int(NULL, &value) == 0);
    return NULL;
}

static const char* test_int_array_contains(void){
    int array[] = {3, 8, 15};
    mu_assert("3 est dans le tableau", int_array_contains(3, 3, array));
    mu_assert("15 est dans le tableau", int_array_contains(15, 3, array));
    mu_assert("4 n'est pas dans le tableau", !int_array_contains(4, 3, array));
    mu_assert("un tableau vide ne contient rien", !int_array_contains(3, 0, array));
    return NULL;
}

static const char* test_max(void){
    mu_assert("max(1, 2) == 2", max(1, 2) == 2);
    mu_assert("max(5, -3) == 5", max(5, -3) == 5);
    mu_assert("max(4, 4) == 4", max(4, 4) == 4);
    return NULL;
}

static const char* test_file_exists(void){
    mu_assert("le dossier include doit exister", file_exists("include", 1) == LABYRINTH_SUCCESS);
    mu_assert("le Makefile doit exister", file_exists("Makefile", 0) == LABYRINTH_SUCCESS);
    mu_assert("un fichier n'est pas un dossier", file_exists("Makefile", 1) == LABYRINTH_NOT_FOUND);
    mu_assert("un dossier n'est pas un fichier", file_exists("include", 0) == LABYRINTH_NOT_FOUND);
    mu_assert("un chemin inexistant doit donner NOT_FOUND", file_exists("chemin_inexistant_xyz", 1) == LABYRINTH_NOT_FOUND);
    return NULL;
}

static const char* test_create_folder_and_list_content(void){
    remove(TEST_FILE);
    rmdir(TEST_FOLDER);

    mu_assert("la création du dossier doit réussir", create_folder(TEST_FOLDER) == LABYRINTH_SUCCESS);
    mu_assert("créer un dossier existant doit donner ALREADY_EXISTS", create_folder(TEST_FOLDER) == LABYRINTH_ALREADY_EXISTS);

    FILE* file = fopen(TEST_FILE, "w");
    mu_assert("création du fichier de test impossible", file != NULL);
    fclose(file);

    char** content = get_folder_content(TEST_FOLDER);
    mu_assert("le contenu du dossier doit être lisible", content != NULL);

    int found = 0;
    int count = 0;
    while(content[count] != NULL){
        if(strcmp(content[count], "fichier.txt") == 0){
            found = 1;
        }
        free(content[count]);
        count++;
    }
    free(content);

    remove(TEST_FILE);
    rmdir(TEST_FOLDER);

    mu_assert("le fichier créé doit être listé", found);
    return NULL;
}

static const char* test_get_folder_content_missing(void){
    mu_assert("un dossier inexistant doit donner NULL", get_folder_content("dossier_inexistant_xyz") == NULL);
    return NULL;
}

static const char* test_read_entry(void){
    FILE* input = tmpfile();
    mu_assert("fichier temporaire impossible", input != NULL);
    fputs("zSq\nx", input);
    fputs("\033[A", input);
    rewind(input);

    // On remplace temporairement l'entrée standard
    int saved_stdin = dup(STDIN_FILENO);
    dup2(fileno(input), STDIN_FILENO);
    clearerr(stdin);

    KeyboardKey keys[6];
    for(int i = 0 ; i < 6 ; i++){
        keys[i] = read_entry();
    }

    fflush(stdin);
    dup2(saved_stdin, STDIN_FILENO);
    close(saved_stdin);
    clearerr(stdin);
    fclose(input);

    mu_assert("'z' doit donner Z", keys[0] == Z);
    mu_assert("'S' doit donner S", keys[1] == S);
    mu_assert("'q' doit donner Q", keys[2] == Q);
    mu_assert("retour à la ligne doit donner ENTER", keys[3] == ENTER);
    mu_assert("un caractère inconnu doit donner NONE", keys[4] == NONE);
    mu_assert("la flèche haut doit donner UP", keys[5] == UP);
    return NULL;
}

void run_utils_tests(void){
    mu_run_test(test_read_int_valid);
    mu_run_test(test_read_int_invalid);
    mu_run_test(test_int_array_contains);
    mu_run_test(test_max);
    mu_run_test(test_file_exists);
    mu_run_test(test_create_folder_and_list_content);
    mu_run_test(test_get_folder_content_missing);
    mu_run_test(test_read_entry);
}
