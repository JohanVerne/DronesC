# Simple makefile for building the drones program

default: drones

drones: drones.c
	gcc -g -Wall drones.c -o drones -lm

clean:
	rm -f *.o drones
