/**
 * Tests unitaires du projet, écrits avec minunit (https://github.com/siu/minunit).
 *
 * minunit.h garde ses compteurs dans des variables static : toutes les suites sont donc
 * incluses ici pour être compilées dans un seul fichier. minunit.h doit rester le premier
 * #include (il définit _POSIX_C_SOURCE avant les en-têtes système).
 */
#include "minunit.h"

#include <stdlib.h>
#include <time.h>

#include "test_labyrinth.h"
#include "test_generator.h"
#include "test_monster.h"
#include "test_utils.h"
#include "test_io.h"

int main(void){
    srand(time(NULL));

    MU_RUN_SUITE(labyrinth_suite);
    MU_RUN_SUITE(generator_suite);
    MU_RUN_SUITE(monster_suite);
    MU_RUN_SUITE(utils_suite);
    MU_RUN_SUITE(io_suite);
    MU_REPORT();

    return MU_EXIT_CODE;
}
