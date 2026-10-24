#ifndef CFG_IO_H
#define CFG_IO_H "cfg_io.h"

#include "labyrinth.h"
#include "utils.h"

#define SAVE_PATH "data/labyrinths/"

/**
 * Enregistre un labyrinthe dans un fichier .cfg dans le dossier SAVE_PATH.
 * Si un labyrinthe avec le même nom existe déjà, la sauvegarde échoue. 
 * 
 * 
 * @param name le nom du labyrinthe. Ce nom sera aussi le nom du fichier dans lequel le labyrinthe est enregistré.
 * @param labyrinth pointeur vers le labyrinthe à enregistrer. 
 * 
 * @return un état représentant le résultat de l'opération de sauvegarde: 
 *  - LABYRINTH_SUCCESS si l'opération est réussie
 *  - LABYRINTH_PERMISSION_ERROR si une erreur de permissions empêche la sauvegarde
 *  - LABYRINTH_CORRUPTED si les paramètres du labyrinthe sont invalides
 *  - LABYRINTH_INVALID_NAME si le nom est invalide
 *  - LABYRINTH_INVALID_FORMAT si le dossier de sauvegarde n'existe pas et 
 *         qu'il est impossible de le créer car un fichier du même nom existe
 *  - LABYRINTH_ALREADY_EXISTS si un labyrinthe avec le même nom existe déjà
 *  - LABYRINTH_UNKNOWN si l'opération échoue pour d'autres raisons
 */
LabyrinthStatus save_labyrinth(char* name, Labyrinth* labyrinth);

/**
 * Permet de charger un labyrinthe depuis un fichier cfg. Ce labyrinthe doit être présent dans le dossier SAVE_PATH.
 * 
 * @param name le nom du labyrinth à charger. Celui-ci doit être le même que celui du fichier cfg dans lequel le labyrinth est enregisré.
 * @param labyrinth pointeur vers le pointeur d'un Labyrinth qui contienadra le labyrinthe chargé. 
 *                  La mémoire occupée par cette variable sera allouée dynamiquement et donc il faut la libérer avec free_labyrinth().
 *                  Si le chargement échoue, la valeur de ce pointeur sera NULL
 * 
 * @return un état représentant le résultat de l'opération de chargement.
 *  - LABYRINTH_IO_SUCCESS si l'opération est réussie
 *  - LABYRINTH_MEMORY_ERROR si une erreur d'allocation de la mémoire empêche le chargement
 *  - LABYRINTH_CORRUPTED si les paramètres du labyrinthe sont invalides
 *  - LABYRINTH_INVALID_NAME si le nom est invalide
 *  - LABYRINTH_NOT_FOUND si aucun labyrinthe avec ce nom n'est trouvé
 *  - LABYRINTH_INVALID_FORMAT si le contenu du fichier est invalide
 *  - LABYRINTH_UNKNOWN si l'opération échoue pour d'autres raisons 
 */
LabyrinthStatus load_labyrinth(char* name, Labyrinth** labyrinth);

/**
 * Permet de construire une chaîne de caractères contenant le chemin vers le fichier de sauvegarde 
 * d'un labyrinthe.
 * 
 * La chaîne de caractères retournée est allouée dynamiquement avec malloc et doit donc être liberé avec free().
 * 
 * @param labyrinth_name le nom dy labyrinthe
 * 
 * @return le chemin vers le fichier de sauvegarde ou nul si l'allocation de la mémoire échoue.
 */
char* build_file_path(char* labyrinth_name);

#endif
