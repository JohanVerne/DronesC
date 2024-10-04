default : test

test : test.c
	gcc -g -Wall test.c -o test -lm