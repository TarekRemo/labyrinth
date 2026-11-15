#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "game.h"
#include "utils.h"
#include "monster.h"

/**
 * Affiche le labyrinthe
 * 
 * @param labyrinth le labyrinthe à afficher
 * 
 * @return void
 */
static void print_labyrinth(Labyrinth labyrinth, long int current_score);

/**
 * Retourne les coordonnées du joueur
 * 
 * @param labyrinth le labyrinthe dans lequel il faut chercher
 * @param coordinates tableau dans lequel il faut mettre les coordoonées. 
 *          Le premier indice correspond à la ligne du joeur et le deuxième correspond à la colonne.
 *          si aucun joueur n'a été trouvé, le tableau est mis à null.
 * 
 * @return void
 */
static void find_player_coordinates(Labyrinth labyrinth, int coordinates[2]);

/**
 * Permet de vérifier si un jour peut se déplacer vers une case.
 * 
 * @param labyrinth le labyrinthe dans lequel il faut vérifier le déplacement
 * @param new_line la ligne de la case vers laquelle le joueur veut se déplacer
 * @param new_col la colonne de la case vers laquelle le joueur veut se déplacer
 * 
 * @return 0 si le déplacement est impossible, 1 sinon.
 */
static int player_can_go_to(Labyrinth* labyrinth, int new_line, int new_col);

/**
 * Déplace tous les monstres et applique les pénalités en cas de contact avec le joueur.
 */
static void move_monsters(Labyrinth* labyrinth, int player_line, int player_col, long int* current_score);

GAMESTATUS launch_game(Labyrinth* labyrinth, long int* nb_mvmts, long int* current_score){
    if(labyrinth == NULL ||labyrinth->cells == NULL){
        return NOT_STARTED;
    }

    set_canonical_mode(0);

    KeyboardKey key;
    int player_coordinates[2];
    find_player_coordinates(*labyrinth, player_coordinates);
    if(player_coordinates[0] == -1){
        return NOT_STARTED;
    }

    int player_line = player_coordinates[0];
    int player_col = player_coordinates[1];
    int new_player_line;
    int new_player_col;
    int game_won;

    // L'écran n'est effacé qu'une fois : les images suivantes sont dessinées par-dessus
    // pour éviter le clignotement.
    set_background_color(GAME_BACKGROUND_COLOR);

    while(1){
        print_labyrinth(*labyrinth, *current_score);
 
        //on attend une entrée valide
        while((key = read_entry()) == ENTER || key == NONE);

        new_player_line = player_line;
        new_player_col = player_col;
        switch(key){
            case UP:
            case Z:
                new_player_line = player_line-1;
                break;

            case DOWN:
            case S:
                new_player_line = player_line+1;
                break;

            case LEFT:
            case Q:
                new_player_col = player_col-1;
                break;
            
            case RIGHT:
            case D:
                new_player_col = player_col+1;
                break;
            
            case R:
                return NOT_FINISHED;

            default:
                continue;
        }

        if(player_can_go_to(labyrinth, new_player_line, new_player_col) == 1){
            (*nb_mvmts)++;
            labyrinth->cells[player_line][player_col] = EMPTY;
            game_won = 0;

            switch(labyrinth->cells[new_player_line][new_player_col]){
                case DOOR:
                    *current_score += max(0, GAME_MAX_SCORE - GAME_PENALTY_PER_MVMT * (*nb_mvmts));
                    game_won = 1;
                    break;
                
                case KEY:
                    labyrinth->door_unlocked = 1;
                    *current_score += max(0, GAME_MAX_SCORE - GAME_PENALTY_PER_MVMT * (*nb_mvmts));
                    break;

                case TREASURE:
                    *current_score += GAME_TREASURE_BONUS;
                    break;

                case TRAP:
                    *current_score = max(0, *current_score - GAME_TRAP_PENALTY);
                    break;
                
                default:
                    break;
            }

            labyrinth->cells[new_player_line][new_player_col] = PLAYER;
            player_line = new_player_line;
            player_col = new_player_col;

            if(labyrinth->monster_count > 0 && labyrinth->monsters != NULL){
                move_monsters(labyrinth, player_line, player_col, current_score);
            }

            if(game_won){
                return WON;
            }
        }
    }
}

static void print_labyrinth(Labyrinth labyrinth, long int current_score){
    // Toute l'image est écrite en une seule fois : on la prépare dans un tampon complet
    // au lieu de laisser le terminal l'afficher ligne par ligne.
    size_t frame_buffer_size = (size_t)labyrinth.height * labyrinth.width * 64 + 4096;
    char* frame_buffer = malloc(frame_buffer_size);
    fflush(stdout);
    if(frame_buffer != NULL){
        setvbuf(stdout, frame_buffer, _IOFBF, frame_buffer_size);
    }

    // Retour en haut à gauche sans effacer l'écran
    printf("\033[H");

    int height = labyrinth.height;
    int width = labyrinth.width;

    Cell cell_content;
    char cell_representation[4];
    Color cell_back_ground_color;
    Color cell_content_color;
    for(int line = 0 ; line < height ; line++){

        for(int col = 0 ; col < width ; col++){
            cell_content = labyrinth.cells[line][col];

            switch(cell_content){
                case EMPTY:
                    strcpy(cell_representation, " ");
                    cell_back_ground_color = GAME_BACKGROUND_COLOR;
                    cell_content_color = GAME_BACKGROUND_COLOR;
                    break;

                case WALL:
                    strcpy(cell_representation, " ");
                    cell_back_ground_color = GAME_WALL_COLOR;
                    cell_content_color = GAME_WALL_COLOR;
                    break;

                case DOOR:
                    strcpy(cell_representation, labyrinth.door_unlocked == 1 ? " _ " : " # ");
                    cell_back_ground_color = GAME_BACKGROUND_COLOR;
                    cell_content_color = GAME_DOOR_COLOR;
                    break;

                case KEY:
                    strcpy(cell_representation, " % ");
                    cell_back_ground_color = GAME_BACKGROUND_COLOR;
                    cell_content_color = GAME_KEY_COLOR;
                    break;

                case TREASURE:
                    strcpy(cell_representation, " $ ");
                    cell_back_ground_color = GAME_BACKGROUND_COLOR;
                    cell_content_color = GAME_TREASURE_COLOR;
                    break;

                case TRAP:
                    strcpy(cell_representation, " X ");
                    cell_back_ground_color = GAME_BACKGROUND_COLOR;
                    cell_content_color = GAME_TRAP_COLOR;
                    break;

                case PLAYER:
                    strcpy(cell_representation, " O ");
                    cell_back_ground_color = GAME_BACKGROUND_COLOR;
                    cell_content_color = GAME_PLAYER_COLOR;
                    break;

                default:
                    strcpy(cell_representation, " ");
                    cell_back_ground_color = GAME_BACKGROUND_COLOR;
                    cell_content_color = GAME_BACKGROUND_COLOR;
                    break;
            }

            int monster_found = 0;
            int player_shares_cell = 0;
            char monster_representation[2] = {0, 0};
            Color monster_color = GAME_OGRE_COLOR;
            if(labyrinth.monster_count > 0 && labyrinth.monsters != NULL){
                for(int i = 0 ; i < labyrinth.monster_count ; i++){
                    if(labyrinth.monsters[i].line == line && labyrinth.monsters[i].col == col){
                        monster_representation[0] = labyrinth.monsters[i].representation;
                        monster_color = labyrinth.monsters[i].representation == GHOST_REPRESENTATION ? GAME_GHOST_COLOR : GAME_OGRE_COLOR;
                        cell_back_ground_color = GAME_BACKGROUND_COLOR;
                        monster_found = 1;

                        if(cell_content == PLAYER){
                            // Joueur et monstre sur la même case : on affiche les deux (" O" puis la lettre du monstre)
                            player_shares_cell = 1;
                            strcpy(cell_representation, " O");
                            cell_content_color = GAME_PLAYER_COLOR;
                        }
                        else{
                            sprintf(cell_representation, " %c ", monster_representation[0]);
                            cell_content_color = monster_color;
                        }
                        break;
                    }
                }
            }

            print_text_inline(
                cell_representation,
                cell_content_color,
                0,
                cell_back_ground_color,
                1,
                monster_found ? 1 : (cell_content == WALL || cell_content == EMPTY ? 3 : 1)
            );

            if(player_shares_cell){
                print_text_inline(monster_representation, monster_color, 0, cell_back_ground_color, 1, 1);
            }
        }

        if(line == 0){
            char score_string[40];
            sprintf(score_string, "Score : %ld", current_score);
            print_text_inline(score_string, PINK, 1, GAME_BACKGROUND_COLOR, 1, 3);
        }

        // Efface les éventuels restes de l'image précédente (ex: score plus court) avec le fond courant
        printf("\033[K\n");
    }

    // Efface ce qui reste sous le labyrinthe puis envoie l'image complète au terminal
    printf("\033[J");
    fflush(stdout);
    if(frame_buffer != NULL){
        setvbuf(stdout, NULL, _IOLBF, 0);
        free(frame_buffer);
    }
}

static void find_player_coordinates(Labyrinth labyrinth, int coordinates[2]){

    for(int line = 0 ; line < labyrinth.height ; line++){
        for(int col = 0 ; col < labyrinth.width ; col++){
            if(labyrinth.cells[line][col] == PLAYER){
                coordinates[0] = line;
                coordinates[1] = col;
                return;
            }
        }
    }

    coordinates[0] = -1;
}

static int player_can_go_to(Labyrinth* labyrinth, int new_line, int new_col){
    if(
        new_line < 0 || new_line >= labyrinth->height ||
        new_col < 0  || new_col >= labyrinth->width ||
        labyrinth->cells[new_line][new_col] == WALL ||
        (labyrinth->cells[new_line][new_col] == DOOR && labyrinth->door_unlocked == 0 )
    ){
        return 0;
    }

    return 1;
}

static void move_monsters(Labyrinth* labyrinth, int player_line, int player_col, long int* current_score){
    if(labyrinth->monster_count <= 0 || labyrinth->monsters == NULL){
        return;
    }

    for(int i = 0 ; i < labyrinth->monster_count ; i++){
        int was_colliding = labyrinth->monsters[i].line == player_line &&
                            labyrinth->monsters[i].col == player_col;
        if(labyrinth->monsters[i].mobility != NULL){
            labyrinth->monsters[i].mobility(labyrinth, &labyrinth->monsters[i]);
        }

        int is_colliding = labyrinth->monsters[i].line == player_line &&
                           labyrinth->monsters[i].col == player_col;
        if(was_colliding || is_colliding){
            *current_score = max(0, *current_score - labyrinth->monsters[i].penalty_points);
        }
    }
}