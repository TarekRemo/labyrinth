CC = gcc
CPPFLAGS = -Iinclude
CFLAGS = -Wall -Wextra -Wpedantic
EXEC = bin/labyrinth.out
TEST_EXEC = bin/tests.out
TEST_SRC = $(wildcard tests/*.c)
TEST_OBJ = $(patsubst tests/%.c,build/tests_%.o,$(TEST_SRC))
OBJ = build/main.o build/generator.o build/labyrinth.o build/io.o build/utils.o build/launcher.o build/game.o build/monster.o

.PHONY: all clean test

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
build/io.o: include/io.h include/labyrinth.h
build/utils.o: include/utils.h
build/launcher.o: include/launcher.h include/labyrinth.h include/utils.h include/game.h include/generator.h include/io.h include/monster.h
build/game.o: include/game.h include/labyrinth.h include/utils.h
build/monster.o: include/monster.h include/labyrinth.h

$(TEST_EXEC): $(TEST_OBJ) $(filter-out build/main.o,$(OBJ))
	$(CC) $(CFLAGS) $^ -o $@

build/tests_%.o: tests/%.c $(wildcard tests/*.h) $(wildcard include/*.h)
	$(CC) $(CFLAGS) $(CPPFLAGS) -Itests -c $< -o $@

test: $(TEST_EXEC)
	./$(TEST_EXEC)

clean: 
	rm -rf build/* bin/*