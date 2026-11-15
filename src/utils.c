#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
#include "utils.h"

/**
 * Couleur d'arrière plan courante (définie par set_background_color), -1 si aucune.
 * Elle est réappliquée après chaque réinitialisation du style pour que le fond
 * reste cohérent partout (fin de ligne, saisie utilisateur, etc).
 */
static int current_background_color = -1;

/**
 * Retourne le code SGR (ANSI) d'une couleur de texte (30-37, 90-97) ou -1 si inconnue.
 */
static int get_text_color_code(Color color);

/**
 * Retourne le code SGR (ANSI) d'une couleur de fond (40-47, 100-107) ou -1 si inconnue.
 */
static int get_background_color_code(Color color);

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
        
        case 'z':
        case 'Z':
            return Z;

        case 's':
        case 'S':
            return S;

        case 'q':
        case 'Q':
            return Q;

        case 'd':
        case 'D':
            return D;
        
        case 'r':
        case 'R':
            return R;
        
        case '\n':
            return ENTER;
    }

    return NONE;
}

int read_string(char* name, int size){
    if(name == NULL){
        return 0;
    }

    char* name_end;
    while(
        fgets(name, size+1, stdin) == NULL ||
        (name_end = strchr(name, '\n')) == NULL
    ){
        char c;
        while((c = getc(stdin)) != '\0' && c != '\n' && c != EOF);
        return 0;
    }
    *name_end = '\0';
    return 1;
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
        if(strcmp(file_struct->d_name, ".") == 0 || strcmp(file_struct->d_name, "..") == 0){
            continue;
        }
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
    temp = realloc(files_array, sizeof(char*) * (nb_files+1));
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

static void print_text_formatted(char* text, Color text_color, int underline, Color highlight_color, int bold, int newline, int width){
    int text_color_code = get_text_color_code(text_color);
    int highlight_color_code = get_background_color_code(highlight_color);

    printf("\033[0%s%s", underline == 1 ? ";4" : "", bold == 1 ? ";1" : "");
    if(text_color_code != -1){
        printf(";%d", text_color_code);
    }
    if(highlight_color_code != -1){
        printf(";%d", highlight_color_code);
    }
    printf("m%*s\033[0m", width, text);

    // Réapplique le fond global pour que la suite (retour à la ligne, saisie...) l'utilise
    if(current_background_color != -1){
        printf("\033[%dm", current_background_color);
    }

    if(newline){
        printf("\n");
    }
}

void set_background_color(Color color){
    int color_code = get_background_color_code(color);
    current_background_color = color_code;

    // Le fond doit être défini AVANT l'effacement, sinon l'écran est effacé avec l'ancien fond
    printf("\033[0m");
    if(color_code != -1){
        printf("\033[%dm", color_code);
    }
    printf("\033[H\033[2J\033[3J");
    fflush(stdout);
}

void print_text(char* text, Color text_color, int underline, Color highlight_color, int bold, int width){
    print_text_formatted(text, text_color, underline, highlight_color, bold, 1, width);
}

void print_text_inline(char* text, Color text_color, int underline, Color highlight_color, int bold, int width){
    print_text_formatted(text, text_color, underline, highlight_color, bold, 0, width);
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

long int max(long int a, long int b){
    return a > b ? a : b;
}

static int get_text_color_code(Color color){
    if(color < BLACK || color > BRIGHT_WHITE){
        return -1;
    }
    return color < 8 ? 30 + color : 90 + (color - 8);
}

static int get_background_color_code(Color color){
    if(color < BLACK || color > BRIGHT_WHITE){
        return -1;
    }
    return color < 8 ? 40 + color : 100 + (color - 8);
}
