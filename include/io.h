#ifndef IO_H
#define IO_H "io.h"

#include "labyrinth.h"
#include "utils.h"

#define LABYRINTHS_SAVE_PATH "data/labyrinths/"
#define SCORES_SAVE_PATH "data/scores/"

#define MAX_SCORES 10
#define MAX_PLAYER_NAME_LENGTH 128

/**
 * Enregistre un labyrinthe dans un fichier .cfg dans le dossier LABYRINTHS_SAVE_PATH.
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
 * Permet de charger un labyrinthe depuis un fichier cfg. Ce labyrinthe doit être présent dans le dossier LABYRINTHS_SAVE_PATH.
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
 * Enregistre le score si celui-ci fait partie des MAX_SCORES meilleurs scores
 * du labyrinthe. Les scores sont classés du plus élevé au plus faible.
 *
 * @param labyrinth_name le nom du labyrinthe auquel associer le score.
 * @param player_name le nom du joueur, sans espace et d'au plus MAX_PLAYER_NAME_LENGTH caractères.
 * @param score le score strictement positif à enregistrer.
 *
 * @return LABYRINTH_SUCCESS si le score est enregistré,
 *         LABYRINTH_NO_CHANGE s'il ne fait pas partie des MAX_SCORES meilleurs scores,
 *         LABYRINTH_INVALID_NAME si le nom du labyrinthe ou du joueur est invalide,
 *         LABYRINTH_INVALID_FORMAT si le score n'est pas positif,
 *         LABYRINTH_NOT_FOUND si le dossier des scores n'existe pas et ne peut pas être lu,
 *         LABYRINTH_PERMISSION_ERROR si l'accès au dossier des scores est refusé,
 *         LABYRINTH_MEMORY_ERROR si la mémoire nécessaire au chemin manque,
 *         ou LABYRINTH_UNKNOWN en cas d'autre erreur d'entrée/sortie.
 */
LabyrinthStatus save_score(char* labyrinth_name, char* player_name, long int score);

/**
 * Charge au plus MAX_SCORES scores depuis le fichier associé au labyrinthe.
 * Les scores sont classés du plus élevé au plus faible. Les cases inutilisées
 * de scores valent -1 et les noms correspondants sont des chaînes vides.
 *
 * @param labyrinth_name le nom du labyrinthe dont les scores doivent être chargés.
 * @param scores tableau de MAX_SCORES éléments qui recevra les scores.
 * @param player_names tableau de MAX_SCORES noms, chacun pouvant contenir
 *        jusqu'à MAX_PLAYER_NAME_LENGTH caractères plus le terminateur nul.
 *
 * @return LABYRINTH_SUCCESS si les scores sont chargés,
 *         LABYRINTH_INVALID_FORMAT si un argument est invalide ou si le fichier
 *         des scores est mal formé,
 *         LABYRINTH_NOT_FOUND si le fichier des scores n'existe pas,
 *         LABYRINTH_PERMISSION_ERROR si l'accès au fichier est refusé,
 *         LABYRINTH_MEMORY_ERROR si la mémoire nécessaire au chemin manque,
 *         ou LABYRINTH_UNKNOWN en cas d'autre erreur d'entrée/sortie.
 */
LabyrinthStatus load_scores(
    char* labyrinth_name,
    long int scores[MAX_SCORES],
    char player_names[MAX_SCORES][MAX_PLAYER_NAME_LENGTH + 1]
);

/**
 * Permet de construire une chaîne de caractères contenant le chemin vers un fichier de sauvegarde.
 * 
 * La chaîne retournée est allouée dynamiquement et doit être libérée avec free().
 * 
 * @param labyrinth_name le nom du labyrinthe.
 * @param save_type le type de fichier de sauvegarde : 
 *        1 pour un fichier de labyrinthe, 2 pour un fichier de scores.
 * 
 * @return le chemin vers le fichier, ou NULL si le type n'est pas reconnu
 *         ou si l'allocation échoue.
 */
char* build_save_file_path(char* labyrinth_name, int save_type);

/**
 * Vérifie qu'un nom de joueur est non vide, ne dépasse pas la longueur maximale
 * et ne contient aucun espace.
 *
 * @param player_name le nom à vérifier.
 *
 * @return 1 si le nom est valide, 0 sinon.
 */
int is_valid_player_name(const char* player_name);

#endif
