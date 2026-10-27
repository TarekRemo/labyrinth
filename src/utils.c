#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <dirent.h>
#include <termio.h>
#include <unistd.h>
#include "utils.h"

/**
 * permet d'obtenir le code ansi de la couleur souhaitée.
 * 
 * @param color la couleur souhaitée.
 * 
 * @return la code de la couleur. Si la couleur n'est pas reconnue, alors NULL est retourné.
 */
static char* get_color_ansi_code(Color color);

int read_int(FILE* input_descriptor, int* var){
    if(input_descriptor == NULL || var == NULL){
        return 0;
    }

    //Lire une ligne entière pour ne pas laisser de caractères en attente dans le flux
    char buffer[READ_INT_BUFFER_SIZE];
    if(fgets(buffer, sizeof(buffer), input_descriptor) == NULL){
        return 0;
    }

    //Si la ligne est trop longue, consommer le reste et refuser l'entrée
    if(strchr(buffer, '\n') == NULL && !feof(input_descriptor)){
        int c;
        while((c = fgetc(input_descriptor)) != '\n' && c != EOF);
        return 0;
    }

    char* end;
    errno = 0;
    long value = strtol(buffer, &end, 10);

    //Aucun chiffre n'a été lu (entrée vide, "abc", "-"...)
    if(end == buffer){
        return 0;
    }

    //Vérifier que la valeur tient dans un int
    if(errno == ERANGE || value < INT_MIN || value > INT_MAX){
        return 0;
    }

    //Seuls des espaces (dont le retour à la ligne) sont autorisés après le nombre
    while(*end != '\0'){
        if(!isspace((unsigned char)*end)){
            return 0;
        }
        end++;
    }

    *var = (int)value;
    return 1;
}

KeyboardKey read_entry(){
    char c = getchar();

    switch(c){
        //Si c'est le caractère de la touche echap
        case 27:
            //D'autres caractères existent
            getchar(); //ignorer le deuxième caractère
            c = getchar(); //le 3ème permet de connaître la touche

            if(c == 'A'){//flèche du haut
                return UP;
            } 
            else if(c == 'B'){//flèche du bas
                return DOWN;
            }
            else if(c == 'D'){//flèche de la gauche
                return LEFT;
            }
            else if(c == 'C'){//flèche de la droite
                return RIGHT;
            }
            break;
        

        case 'r':
            return R;
        
        case '\n':
            return ENTER;
    }

    return NONE;
}

LabyrinthStatus file_exists(char* folder_path, int is_folder){
    struct stat st = {0};   
    if (stat(folder_path, &st) == -1) {
        switch(errno){
            case ENOENT: //Le fichier n'existe pas
                return LABYRINTH_NOT_FOUND;
            
            case EACCES: //erreur de permission
                return LABYRINTH_PERMISSION_ERROR;
            
            default: //autres erreurs
                return LABYRINTH_UNKNOWN;
        }
    }

    //véifier que c'est le type attendu
    int expected_type = is_folder == S_ISDIR(st.st_mode);

    return expected_type == 1 ? LABYRINTH_SUCCESS : LABYRINTH_NOT_FOUND;
}

LabyrinthStatus create_folder(char* folder_path){
    if(mkdir(folder_path, 0700) == -1){
        switch(errno){
            case EACCES:
                return LABYRINTH_PERMISSION_ERROR;
            
            case EEXIST:
                return LABYRINTH_ALREADY_EXISTS;

            default:
                return LABYRINTH_UNKNOWN;
        }
    }
    return LABYRINTH_SUCCESS;
}

char** get_folder_content(char* folder_path){   
    DIR* folder;
    struct dirent* file_struct;

    folder = opendir (folder_path);
    if(folder == NULL){
        return NULL;
    }

    int nb_files = 0;
    char** files_array = NULL;
    char** temp = NULL;
    while ((file_struct = readdir(folder))){
        nb_files++;

        temp = realloc(files_array, sizeof(char*) * nb_files);
        if(temp == NULL){
            if(nb_files > 1){    
                for(int i = 0 ; i < nb_files-1 ; i++){
                    free(files_array[i]);
                }
                free(files_array);
            }
            return NULL;
        }

        files_array = temp;

        files_array[nb_files-1] = malloc(sizeof(file_struct->d_name));
        if(files_array[nb_files-1] == NULL){
            for(int i = 0 ; i < nb_files-1 ; i++){
                free(files_array[i]);
            }
            free(files_array);
            return NULL;
        }

        strcpy(files_array[nb_files-1], file_struct->d_name);
    }

    //ajouter une dernière case NULL indiquant la fin de la liste
    temp = realloc(files_array, sizeof(char*) * nb_files+1);
    if(temp == NULL){   
        for(int i = 0 ; i < nb_files ; i++){
            free(files_array[i]);
        }
        free(files_array);
        return NULL;
    }

    files_array = temp;
    files_array[nb_files] = NULL;
    
    return files_array;
}

void set_background_color(Color color){
    char* color_code = get_color_ansi_code(color);
    printf("\033[48;5;%sm\033[2J\033[H", color_code);
}

void print_text(char* text, Color text_color, int underline, Color highlight_color, int bold){
    char* text_color_code = get_color_ansi_code(text_color);
    text_color_code = text_color_code != NULL ? text_color_code : "";

    char* underline_code = underline == 1 ? "4" : "";

    char* highlight_color_code = get_color_ansi_code(highlight_color);
    highlight_color_code = highlight_color_code != NULL ? highlight_color_code : "";

    char* bold_code = bold == 1 ? "1" : "";

    fprintf(
        stdout,
        "\033[%s;%s;38;5;%s;48;5;%sm%s\033[0m\n",
        underline_code,
        bold_code,
        text_color_code,
        highlight_color_code,
        text
    );
}

void set_canonical_mode(int canonical){
    struct termios oldt; 
    tcgetattr(STDIN_FILENO, &oldt); 
    if(canonical == 0)
        oldt.c_lflag &= ~(ICANON | ECHO); 
    else if (canonical == 1)
        oldt.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int int_array_contains(int value, int array_size, int* array){
    for(int i = 0 ; i < array_size ; i++){
        if(array[i] == value)
            return 1;
    }
    return 0;
}

static char* get_color_ansi_code(Color color){
    switch(color){
        case GRAY:
            return "250";
        case GREEN:
            return "40";
        case PINK:
            return "200";
        case BLACK:
            return "0";
        case RED:
            return "9";
        default:
            return NULL;
    }
}
