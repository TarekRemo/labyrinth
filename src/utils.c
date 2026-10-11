#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
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

int file_exists(char* file_path){
    struct stat st = {0};
    int result;
    if (stat(file_path, &st) == -1) {
        switch(errno){
            case ENOENT: //Le fichier n'existe pas
                result = 0;
                break;
            
            default: //Une autre erreur signifie que le fichier existe
                result = 1;
                break;
        }
    }else{
        result = 1; //pas d'erreur, donc le fichier existe
    }

    return result;
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

    printf(
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
        default:
            return NULL;
    }
}
