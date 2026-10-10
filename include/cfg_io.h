#ifndef CFG_IO_H
#define CFG_IO_H "cfg_io.h"

#include "labyrinth.h"

#define SAVE_PATH "data/labyrinths/"

/**
 * Enum représentant l'état de retour des opérations d'enregistrement/chargement de labyrinthes
 */
typedef enum LabyrinthIOStatus{
    LABYRINTH_IO_SUCCESS,
    LABYRINTH_IO_PERMISSION_ERROR,
    LABYRINTH_IO_NOT_FOUND,
    LABYRINTH_IO_CORRUPTED,
    LABYRINTH_IO_INVALID_NAME,
    LABYRINTH_IO_INVALID_FORMAT,
    LABYRINTH_IO_ALREADY_EXISTS,
    LABYRINTH_IO_MEMORY_ERROR,
    LABYRINTH_IO_UNKNOWN
}LabyrinthIOStatus;

/**
 * Enregistre un labyrinthe dans un fichier .cfg dans le dossier SAVE_PATH.
 * Si un labyrinthe avec le même nom existe déjà, la sauvegarde échoue. 
 * 
 * 
 * @param name le nom du labyrinthe. Ce nom sera aussi le nom du fichier dans lequel le labyrinthe est enregistré.
 * @param labyrinth pointeur vers le labyrinthe à enregistrer. 
 * 
 * @return LabyrinthIOStatus un état représentant le résultat de l'opération de sauvegarde: 
 *  - LABYRINTH_IO_SUCCESS si l'opération est réussie
 *  - LABYRINTH_IO_PERMISSION_ERROR si une erreur de permissions empêche la sauvegarde
 *  - LABYRINTH_IO_CORRUPTED si les paramètres du labyrinthe sont invalides
 *  - LABYRINTH_IO_INVALID_NAME si le nom est invalide
 *  - LABYRINTH_IO_INVALID_FORMAT si le dossier de sauvegarde n'existe pas et 
 *         qu'il est impossible de le créer car un fichier du même nom existe
 *  - LABYRINTH_IO_ALREADY_EXISTS si un labyrinthe avec le même nom existe déjà
 *  - LABYRINTH_IO_UNKNOWN si l'opération échoue pour d'autres raisons
 */
LabyrinthIOStatus save_labyrinth(char* name, Labyrinth* labyrinth);

/**
 * Permet de charger un labyrinthe depuis un fichier cfg. Ce labyrinthe doit être présent dans le dossier SAVE_PATH.
 * 
 * @param name le nom du labyrinth à charger. Celui-ci doit être le même que celui du fichier cfg dans lequel le labyrinth est enregisré.
 * @param labyrinth pointeur vers le pointeur d'un Labyrinth qui contienadra le labyrinthe chargé. 
 *                  La mémoire occupée par cette variable sera allouée dynamiquement et donc il faut la libérer avec free_labyrinth().
 *                  Si le chargement échoue, la valeur de ce pointeur sera NULL
 * 
 * @return LabyrinthIOStatus un état représentant le résultat de l'opération de chargement. 
 */
LabyrinthIOStatus load_labyrinth(char* name, Labyrinth** labyrinth);

#endif
