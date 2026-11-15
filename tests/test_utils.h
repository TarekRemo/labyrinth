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

MU_TEST(test_read_int_valid){
    int value = 0;
    mu_assert(read_int_from_text("42\n", &value) == 1 && value == 42, "un entier valide doit être lu");
    mu_assert(read_int_from_text("-7\n", &value) == 1 && value == -7, "un entier négatif doit être lu");
}

MU_TEST(test_read_int_invalid){
    int value = 0;
    mu_assert(read_int_from_text("abc\n", &value) == 0, "du texte doit être refusé");
    mu_assert(read_int_from_text("12abc\n", &value) == 0, "un nombre suivi de texte doit être refusé");
    mu_assert(read_int_from_text("\n", &value) == 0, "une ligne vide doit être refusée");
    mu_assert(read_int_from_text("99999999999999\n", &value) == 0, "un nombre trop grand doit être refusé");
    mu_assert(read_int(NULL, &value) == 0, "NULL doit être refusé");
}

MU_TEST(test_int_array_contains){
    int array[] = {3, 8, 15};
    mu_assert(int_array_contains(3, 3, array), "3 est dans le tableau");
    mu_assert(int_array_contains(15, 3, array), "15 est dans le tableau");
    mu_assert(!int_array_contains(4, 3, array), "4 n'est pas dans le tableau");
    mu_assert(!int_array_contains(3, 0, array), "un tableau vide ne contient rien");
}

MU_TEST(test_max){
    mu_assert(max(1, 2) == 2, "max(1, 2) == 2");
    mu_assert(max(5, -3) == 5, "max(5, -3) == 5");
    mu_assert(max(4, 4) == 4, "max(4, 4) == 4");
}

MU_TEST(test_file_exists){
    mu_assert(file_exists("include", 1) == LABYRINTH_SUCCESS, "le dossier include doit exister");
    mu_assert(file_exists("Makefile", 0) == LABYRINTH_SUCCESS, "le Makefile doit exister");
    mu_assert(file_exists("Makefile", 1) == LABYRINTH_NOT_FOUND, "un fichier n'est pas un dossier");
    mu_assert(file_exists("include", 0) == LABYRINTH_NOT_FOUND, "un dossier n'est pas un fichier");
    mu_assert(file_exists("chemin_inexistant_xyz", 1) == LABYRINTH_NOT_FOUND, "un chemin inexistant doit donner NOT_FOUND");
}

MU_TEST(test_create_folder_and_list_content){
    remove(TEST_FILE);
    rmdir(TEST_FOLDER);

    mu_assert(create_folder(TEST_FOLDER) == LABYRINTH_SUCCESS, "la création du dossier doit réussir");
    mu_assert(create_folder(TEST_FOLDER) == LABYRINTH_ALREADY_EXISTS, "créer un dossier existant doit donner ALREADY_EXISTS");

    FILE* file = fopen(TEST_FILE, "w");
    mu_assert(file != NULL, "création du fichier de test impossible");
    fclose(file);

    char** content = get_folder_content(TEST_FOLDER);
    mu_assert(content != NULL, "le contenu du dossier doit être lisible");

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

    mu_assert(found, "le fichier créé doit être listé");
}

MU_TEST(test_get_folder_content_missing){
    mu_assert(get_folder_content("dossier_inexistant_xyz") == NULL, "un dossier inexistant doit donner NULL");
}

MU_TEST(test_read_entry){
    FILE* input = tmpfile();
    mu_assert(input != NULL, "fichier temporaire impossible");
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

    mu_assert(keys[0] == Z, "'z' doit donner Z");
    mu_assert(keys[1] == S, "'S' doit donner S");
    mu_assert(keys[2] == Q, "'q' doit donner Q");
    mu_assert(keys[3] == ENTER, "retour à la ligne doit donner ENTER");
    mu_assert(keys[4] == NONE, "un caractère inconnu doit donner NONE");
    mu_assert(keys[5] == UP, "la flèche haut doit donner UP");
}

MU_TEST_SUITE(utils_suite){
    MU_RUN_TEST(test_read_int_valid);
    MU_RUN_TEST(test_read_int_invalid);
    MU_RUN_TEST(test_int_array_contains);
    MU_RUN_TEST(test_max);
    MU_RUN_TEST(test_file_exists);
    MU_RUN_TEST(test_create_folder_and_list_content);
    MU_RUN_TEST(test_get_folder_content_missing);
    MU_RUN_TEST(test_read_entry);
}
