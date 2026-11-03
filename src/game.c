#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "utils.h"

/**
 * Affiche le labyrinthe
 * 
 * @param labyrinth le labyrinthe à afficher
 * 
 * @return void
 */
static void print_labyrinth(Labyrinth labyrinth);

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

GAMESTATUS launch_game(Labyrinth* labyrinth){
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
    while(1){
        print_labyrinth(*labyrinth);
 
        //on attend une entrée valide
        while((key = read_entry()) == ENTER || key == NONE);

        new_player_line = player_line;
        new_player_col = player_col;
        switch(key){
            case UP:
                new_player_line = player_line-1;
                break;

            case DOWN:
                new_player_line = player_line+1;
                break;

            case LEFT:
                new_player_col = player_col-1;
                break;
            
            case RIGHT:
                new_player_col = player_col+1;
                break;
            
            case R:
                return NOT_FINISHED;

            default:
                continue;
        }

        if(player_can_go_to(labyrinth, new_player_line, new_player_col) == 1){
            labyrinth->cells[player_line][player_col] = EMPTY;

            if(labyrinth->cells[new_player_line][new_player_col] == DOOR){
                return WON;
            }

            labyrinth->cells[new_player_line][new_player_col] = PLAYER;
            player_line = new_player_line;
            player_col = new_player_col;
        }
    }
}

static void print_labyrinth(Labyrinth labyrinth){
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
                    strcpy(cell_representation, " _ ");
                    cell_back_ground_color = GAME_BACKGROUND_COLOR;
                    cell_content_color = GAME_DOOR_COLOR;
                    break;

                case KEY:
                    strcpy(cell_representation, " ^ ");
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
        labyrinth->cells[new_line][new_col] == WALL
    ){
        return 0;
    }

    return 1;
}