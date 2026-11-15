#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "minunit.h"

int mu_tests_run = 0;
int mu_tests_failed = 0;

void run_labyrinth_tests(void);
void run_generator_tests(void);
void run_monster_tests(void);
void run_utils_tests(void);
void run_io_tests(void);

int main(void){
    srand(time(NULL));

    printf("== labyrinth ==\n");
    run_labyrinth_tests();
    printf("== generator ==\n");
    run_generator_tests();
    printf("== monster ==\n");
    run_monster_tests();
    printf("== utils ==\n");
    run_utils_tests();
    printf("== io ==\n");
    run_io_tests();

    printf("\n%d test(s) exécuté(s), %d échec(s)\n", mu_tests_run, mu_tests_failed);
    return mu_tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
