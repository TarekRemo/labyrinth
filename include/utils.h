#ifndef UTILS_H
#define UTILS_H "utils.h"

#include <stdio.h>

#define READ_INT_BUFFER_SIZE 64
#define COLOR_ANSI_CODE_BUFFER_SIZE 64

/**
 * Enum représentant les couleurs
 */
typedef enum Color{
    GRAY,
    GREEN,
    PINK,
    BLACK,
    RED
} Color;

/**
 * Enum représentant les touches clavier que l'utilisateur peut taper.
 */
typedef enum KeyboardKey{
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    R, 
    ENTER
} KeyboardKey;

/**
 * Enum représentant l'état de retour des opérations sur labyrinthes (génération, enregistrement, chargement, etc)
 */
typedef enum LabyrinthStatus{
    LABYRINTH_SUCCESS,
    LABYRINTH_PERMISSION_ERROR,
    LABYRINTH_NOT_FOUND,
    LABYRINTH_CORRUPTED,
    LABYRINTH_INVALID_NAME,
    LABYRINTH_INVALID_FORMAT,
    LABYRINTH_ALREADY_EXISTS,
    LABYRINTH_MEMORY_ERROR,
    LABYRINTH_UNKNOWN
}LabyrinthStatus;

/**
 * Vérifie et lit un nombre entier depuis le descripteur input_descriptor et le stock dans var.
 * 
 * @param input_descriptor le descipteur du fichier source.
 * @param var pointeur vers la variable qui va contenir la valeur lue.
 * 
 * @return 1 si l'opération est réussie, 0 sinon.
 */
int read_int(FILE* input_descriptor, int* var);

/**
 * Lit un caractère depuis l'entrée standard et retourne la valeur de KeyboardKey correspondant.
 * 
 * @return l'élément de KeyboardKey correspondant au caractère entré. Si le caractère n'est pas reconnu, alors NONE est retourné.
 */
KeyboardKey read_entry(void);

/**
 * Permet de vérifier si un fichier/dossier existe et est accessible.
 * 
 * @param file_path le chemin du fichier à chercher.
 * @param is_folder 1 si le fichier recherché est un dossier, 0 sinon.
 * 
 * @return LABYRINTH_SUCCESS si le fichier existe et est accessible, LABYRINTH_NOT_FOUND sinon et LABYRINTH_PERMISSION_ERROR si il n'est pas accessible.
 */
LabyrinthStatus file_exists(char* folder_path, int is_folder);

/**
 * Permet de créer un dossier
 * 
 * @param folder_path le chemin du dossier à créer
 * 
 * @return LABYRINTH_SUCCESS si le dossier est crée avec succès,
 *  LABYRINTH_PERMISSION_ERROR, LABYRINTH_ALREADY_EXISTS ou LABYRINTH_UNKNOWN si la création échoue.
 */
LabyrinthStatus create_folder(char* folder_path);

/**
 * Retourne la liste de fichiers/sous-dossiers dans un dossier.
 * La mémoire occupée par le tableau retourné et ses éléments est allouée dynamiquement et donc devra être liberée avec free().
 * 
 * @param folder_path le chemin du dossier
 * 
 * @return la liste des fichiers/sous-dossiers terminée par un élément NULL ou NULL si le dossier n'existe pas ou si la lecture est impossible.
 */
char** get_folder_content(char* folder_path);

/**
 * Permet d'effacer le contenu affiché et de changer la couleur d'arrière plan du terminal.
 * 
 * @param color la couleur souhaitée.
 * 
 * @return void
 */
void set_background_color(Color color);

/**
 * Permet d'afficher un texte en appliquant un style spécifique (couleur, surlignage, etc).
 * Un retour à la ligne est automatiquement appliqué à la fin.
 * 
 * @param text le texte à afficher.
 * @param text_color la couleur du text.
 * @param underline 1 s'il faut souligner, 0 sinon.
 * @param highlight_color couleur de surlignage.
 * @param bold 1 pour afficher en gras, 0 sinon.
 * 
 * @return void
 */
void print_text(char* text, Color text_color, int underline, Color highlight_color, int bold);

/**
 * Configure le mode d'entrée du terminal pour le descripteur de l'entrée standard.
 * 
 * Deux modes sont possibles : 
 *  1 - canonique (= Un retour à la ligne est nécessaire pour valider l'entrée).
 *  2 - non canonique (= Les caractères sont directement transmis au programme sans retour à la ligne nécessaire).
 *
 * @param canonical 1 pour activer le mode canonique, 0 pour activer le mode non canonique.
 * 
 * @return void
 */
void set_canonical_mode(int canonical);

/**
 * Permet de vérifier si un tableau d'entier contient un caractère.
 * 
 * @param value la valeur à rechercher
 * @param array_size la taille du tableau array
 * @param array le tableau dans lequel il faut chercher
 * 
 * @return 1 si la valeur est présente, 0 sinon.
 */
int int_array_contains(int value, int array_size, int* array);

#endif
