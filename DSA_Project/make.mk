build: quadtree.o main.o
	gcc quadtree.o main.o -o quadtree

quadtree.o: quadtree.c quadtree.h
	gcc -c quadtree.c

main.o: main.c
	gcc -c main.c