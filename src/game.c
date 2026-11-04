#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "game.h"
#include "utils.h"

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

GAMESTATUS launch_game(Labyrinth* labyrinth, long int* elapsed_time, long int* current_score){
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

    int start_time = time(NULL);
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
                //arrêter le chrono
                *elapsed_time += time(NULL)-start_time;
                return NOT_FINISHED;

            default:
                continue;
        }

        if(player_can_go_to(labyrinth, new_player_line, new_player_col) == 1){
            labyrinth->cells[player_line][player_col] = EMPTY;

            switch(labyrinth->cells[new_player_line][new_player_col]){
                case DOOR:
                    *elapsed_time += time(NULL)-start_time;
                    *current_score += max(0, GAME_MAX_SCORE - GAME_PENALTY_PER_SEC * (*elapsed_time));
                    return WON;
                
                case KEY:
                    *elapsed_time += time(NULL)-start_time;
                    labyrinth->door_unlocked = 1;
                    *current_score += max(0, GAME_MAX_SCORE - GAME_PENALTY_PER_SEC * (*elapsed_time));
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
        }
    }
}

static void print_labyrinth(Labyrinth labyrinth, long int current_score){
    set_background_color(GAME_BACKGROUND_COLOR);

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

            print_text_inline(
                cell_representation,
                cell_content_color,
                0,
                cell_back_ground_color,
                1,
                cell_content == WALL || cell_content == EMPTY ? 3  : 1
            );
        }

        if(line == 0){
            char score_string[40];
            sprintf(score_string, "Score : %ld", current_score);
            print_text_inline(score_string, PINK, 1, BLACK, 1, 3);
        }

        printf("\n");
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