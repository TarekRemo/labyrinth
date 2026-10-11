CC = gcc
CPPFLAGS = -Iinclude
CFLAGS = -Wall -Wextra -Wpedantic
EXEC = bin/labyrinth.out
OBJ = build/main.o build/generator.o build/labyrinth.o build/cfg_io.o build/utils.o build/launcher.o

.PHONY: all clean

all: $(EXEC)

# construction de l'exécutable 
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# Construction des fichiers .o à partir des .c
build/%.o: src/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Dépendnaces des autres fichiers .o
build/generator.o: include/generator.h include/labyrinth.h
build/labyrinth.o: include/labyrinth.h
build/cfg_io.o: include/cfg_io.h include/labyrinth.h
build/utils.o: include/utils.h
build/launcher.o: include/launcher.h include/labyrinth.h include/utils.h

clean: 
	rm -rf build/* bin/*