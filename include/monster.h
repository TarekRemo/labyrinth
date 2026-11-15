#ifndef MONSTER_H
#define MONSTER_H "monster.h"

#include "labyrinth.h"
#include "utils.h"

#define CELLS_PER_GHOST 100
#define CELLS_PER_OGRE 100

#define GHOST_PENALITY 3
#define OGRE_PENALITY 5

#define GHOST_REPRESENTATION 'F'
#define OGRE_REPRESENTATION 'G'


/**
 * Place les monstres sur des cases libres du labyrinthe.
 *
 * @param labyrinth pointeur vers le labyrinthe
 *
 * @return LABYRINTH_NO_CHANGE si le labyrinthe est trop petit pour avoir des monstres,
 *         LABYRINTH_MEMORY_ERROR si l'allocation de mémoire est impossible
 *         LABYRINTH_MEMORY_SUCCESS si l'opération réussie
 */
LabyrinthStatus add_random_monsters(Labyrinth* labyrinth);

/**
 * Déplace un fantôme dans la limite de son étendue de mobilité, sans tenir compte des murs.
 *
 * @param labyrinth le labyrinthe contenant le monstre
 * @param monster le fantôme à déplacer
 *
 * @return void
 */
void move_ghost(Labyrinth* labyrinth, Monster* monster);

/**
 * Déplace un ogre dans les couloirs, en le rapprochant des trésors.
 *
 * @param labyrinth le labyrinthe contenant le monstre
 * @param monster l'ogre à déplacer
 *
 * @return void
 */
void move_ogre(Labyrinth* labyrinth, Monster* monster);

#endif