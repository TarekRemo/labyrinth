#ifndef LABYRINTH_H
#define LABYRINTH_H "labyrinth.h"

/**
 * Enum permettant de représenter une cellule du labyrinth
 * Une cellule peut être vide ou contenir un mur, une porte, une clé ou un piège
 */
typedef enum Cell {
    UNKNOWN,
    EMPTY,
    WALL,
    DOOR,
    KEY,
    TREASURE,
    TRAP,
    PLAYER
} Cell;

/**
 * Enum permettant de représenter la difficulté du labyrinth
 */
typedef enum Difficulty {
    EASY,
    HARD
} Difficulty;

/**
 * Structure représentant un labyrinthe
 * @var int height la hauteur du labyrinthe
 * @var int width la largeur du labyrinthe
 * @var Difficulty la difficulté du labyrinthe
 * @var cell** cells un tableau 2D contenant les cellules du labyrinthe             
 */
typedef struct Labyrinth {
    int height;
    int width;
    Difficulty difficulty;
    Cell** cells;
} Labyrinth;

/**
 * Initialise et retourne un pointeur vers un labyrinthe. 
 * 
 * La mémoire est allouée dynamiquement et doit donc être libérér manuellement
 *  en utilisant free_labyrinth().
 * 
 * @param difficulty la difficulté du labyrinthe 
 * @param height la hauteur du labyrinthe 
 * @param width la largeur du labyrinthe 
 * 
 * @return un pointeur Labyrinth* si l'allocation réussie, NULL sinon.
 */
Labyrinth* allocate_labyrinth(int difficulty, int height, int width);

/**
 * Permet de libérer la mémoire alloué pour un labyrinthe
 * 
 * @param labyrinth pointeur vers le labyrinthe à libérer
 * 
 * @return void
 */
void free_labyrinth(Labyrinth* labyrinth);

/**
 * Permet de vérifier si les paramètres d'un labyrinthe sont valides.
 * 
 * @param difficulty la difficulté d'un labyrinthe
 * @param height la hauteur du labyrinthe
 * @param width la largeur du labyrinthe
 * 
 * @return 1 si les paramètres sont valides, 0 sinon.
 */
int is_valid_labyrinth_params(Difficulty difficulty, int height, int width);

/**
 * Permet d'obtenir le caractère représentant le contenu d'une cellule.
 * 
 * @param content le contenu de la cellule.
 * 
 * @return le caractère représentant graphiquement cette cellule.
 */
char get_cell_representation(Cell content);

/**
 * Permet d'obtenir la valeur de Cell représentée par un caractère spécifique.
 * 
 * @param c le caractère représentant la cellule.
 * 
 * @return Cell correspondant à ce caractère.
 */
Cell get_cell(char c);

#endif
