CC=gcc
CFLAGS=-I.


parallel_sum : utils.o find_min_max.o utils.h find_min_max.h  parallel_min_max.c
	$(CC) -o parallel_sum utils.o parallel_sum.c $(CFLAGS)
	
parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h  parallel_min_max.c
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

zombie: zombie.c
	$(CC) zombie.c -o zombie 

process_memory: process_memory.c
	$(CC) process_memory.c -o process_memory
clean :
	rm utils.o find_min_max.o parallel_min_max

all : parallel_min_max