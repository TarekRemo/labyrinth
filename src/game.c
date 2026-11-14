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
            move_monsters(labyrinth, player_line, player_col, current_score);
            if(game_won){
                return WON;
            }
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

            int monster_found = 0;
            for(int i = 0 ; i < labyrinth.monster_count ; i++){
                if(labyrinth.monsters[i].line == line && labyrinth.monsters[i].col == col){
                    sprintf(cell_representation, " %c ", labyrinth.monsters[i].representation);
                    cell_back_ground_color = GAME_BACKGROUND_COLOR;
                    cell_content_color = labyrinth.monsters[i].representation == 'F' ? PURPLE : RED;
                    monster_found = 1;
                    break;
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

void move_ghost(Labyrinth* labyrinth, Monster* monster){
    int candidate_count = 0;
    int selected_line = monster->line;
    int selected_col = monster->col;

    for(int line = 0 ; line < labyrinth->height ; line++){
        for(int col = 0 ; col < labyrinth->width ; col++){
            int distance = abs(line - monster->line) + abs(col - monster->col);
            if(distance > monster->penalty_points ||
               labyrinth->cells[line][col] == WALL ||
               (labyrinth->cells[line][col] == DOOR && labyrinth->door_unlocked == 0)){
                continue;
            }

            int occupied = 0;
            for(int i = 0 ; i < labyrinth->monster_count ; i++){
                if(&labyrinth->monsters[i] != monster &&
                   labyrinth->monsters[i].line == line &&
                   labyrinth->monsters[i].col == col){
                    occupied = 1;
                    break;
                }
            }
            if(occupied){
                continue;
            }

            candidate_count++;
            if(rand() % candidate_count == 0){
                selected_line = line;
                selected_col = col;
            }
        }
    }

    monster->line = selected_line;
    monster->col = selected_col;
}

void move_ogre(Labyrinth* labyrinth, Monster* monster){
    static const int line_offsets[] = {-1, 1, 0, 0};
    static const int col_offsets[] = {0, 0, -1, 1};

    for(int step = 0 ; step < monster->penalty_points ; step++){
        int target_line = -1;
        int target_col = -1;
        int current_treasure_distance = labyrinth->height + labyrinth->width;
        int selected_line = monster->line;
        int selected_col = monster->col;
        int candidate_count = 0;

        for(int line = 0 ; line < labyrinth->height ; line++){
            for(int col = 0 ; col < labyrinth->width ; col++){
                if(labyrinth->cells[line][col] == TREASURE){
                    int distance = abs(line - monster->line) + abs(col - monster->col);
                    if(distance < current_treasure_distance){
                        current_treasure_distance = distance;
                        target_line = line;
                        target_col = col;
                    }
                }
            }
        }

        if(target_line != -1 && current_treasure_distance <= 1){
            break;
        }

        for(int direction = 0 ; direction < 4 ; direction++){
            int new_line = monster->line + line_offsets[direction];
            int new_col = monster->col + col_offsets[direction];
            if(new_line < 0 || new_line >= labyrinth->height ||
               new_col < 0 || new_col >= labyrinth->width ||
               labyrinth->cells[new_line][new_col] == WALL ||
               labyrinth->cells[new_line][new_col] == TREASURE ||
               (labyrinth->cells[new_line][new_col] == DOOR && labyrinth->door_unlocked == 0)){
                continue;
            }

            int occupied = 0;
            for(int i = 0 ; i < labyrinth->monster_count ; i++){
                if(&labyrinth->monsters[i] != monster &&
                   labyrinth->monsters[i].line == new_line &&
                   labyrinth->monsters[i].col == new_col){
                    occupied = 1;
                    break;
                }
            }
            if(occupied){
                continue;
            }

            if(target_line == -1){
                candidate_count++;
                if(rand() % candidate_count == 0){
                    selected_line = new_line;
                    selected_col = new_col;
                }
            }
            else{
                int distance = abs(target_line - new_line) + abs(target_col - new_col);
                int selected_distance = abs(target_line - selected_line) + abs(target_col - selected_col);
                if(distance < selected_distance){
                    selected_line = new_line;
                    selected_col = new_col;
                    candidate_count = 1;
                }
                else if(distance == selected_distance && rand() % ++candidate_count == 0){
                    selected_line = new_line;
                    selected_col = new_col;
                }
            }
        }

        if(selected_line == monster->line && selected_col == monster->col){
            break;
        }
        monster->line = selected_line;
        monster->col = selected_col;
    }
}

static void move_monsters(Labyrinth* labyrinth, int player_line, int player_col, long int* current_score){
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