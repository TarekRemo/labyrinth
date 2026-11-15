#ifndef MINUNIT_H
#define MINUNIT_H

/**
 * Mini framework de tests unitaires inspiré de MinUnit.
 *
 * Un test est une fonction `const char* nom(void)` qui retourne NULL en cas de succès
 * ou un message d'erreur (renvoyé par mu_assert) en cas d'échec.
 * Les compteurs sont définis dans tests/test_main.c.
 */

#include <stdio.h>

extern int mu_tests_run;
extern int mu_tests_failed;

#define mu_assert(message, test) do { if(!(test)) return message; } while(0)

#define mu_run_test(test) do { \
    const char* mu_message = test(); \
    mu_tests_run++; \
    if(mu_message){ \
        mu_tests_failed++; \
        printf("  [ECHEC] %s : %s\n", #test, mu_message); \
    } \
    else{ \
        printf("  [OK]    %s\n", #test); \
    } \
} while(0)

#endif
