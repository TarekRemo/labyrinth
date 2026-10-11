#ifndef UTILS_H
#define UTILS_H "utils.h"

#include <stdio.h>

#define READ_INT_BUFFER_SIZE 64
#define COLOR_ANSI_CODE_BUFFER_SIZE 64

/**
 * Enum représentant les couleurs
 */
typedef enum Color{
    NONE,
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
    ESCAPE, 
    ENTER
} KeyboardKey;

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
 * Permet de vérifier si un fichier existe.
 * 
 * @param file_path le chemin du fichier à chercher.
 * 
 * @return 1 si le fichier existe, 0 sinon.
 */
int file_exists(char* file_path);

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

#endif
