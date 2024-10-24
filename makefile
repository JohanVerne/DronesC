# Simple makefile for building the drones program

# Commandes de compilation
all: deps main

<<<<<<< HEAD
main: drones.c window.c
	gcc -g -Wall -o Main drones.c window.c -lm -lSDL2 -lSDL2_image
=======
main: drones.c
	gcc -g -Wall -o Main drones.c -lm -lSDL2 -lSDL2_image
>>>>>>> test

# Cible pour installer les dépendances
deps:
	@echo "Installing SDL2 and SDL2_image dependencies..."
	sudo apt-get install -y libsdl2-dev libsdl2-image-dev

clean:
	rm -f *.o
