#ifndef UTILS_H
#define UTILS_H "utils.h"

#include <stdio.h>

#define READ_INT_BUFFER_SIZE 64

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
 * Permet de vérifier si un fichier existe.
 * 
 * @param file_path le chemin du fichier à chercher.
 * 
 * @return 1 si le fichier existe, 0 sinon.
 */
int file_exists(char* file_path);

#endif
