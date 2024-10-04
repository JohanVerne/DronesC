default : drones

test : drones.c
	gcc -g -Wall drones.c -o drones -lm