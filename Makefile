CC = h5pcc
CFLAGS = -I./include -Wall -O3 -std=c99
LIBS = -lfftw3_mpi -lfftw3 -lm

main: obj/state.o obj/main.o obj/io.o obj/dynamics.o
	$(CC) obj/* $(CFLAGS) $(LIBS) -o main

obj/main.o: src/main.c
	$(CC) -c src/main.c $(CFLAGS) $(LIBS) -o obj/main.o

obj/state.o: src/state.c include/state.h
	$(CC) -c src/state.c $(CFLAGS) $(LIBS) -o obj/state.o

obj/io.o: src/io.c include/io.h
	$(CC) -c src/io.c $(CFLAGS) $(LIBS) -o obj/io.o

obj/dynamics.o: src/dynamics.c include/dynamics.h
	$(CC) -c src/dynamics.c $(CFLAGS) $(LIBS) -o obj/dynamics.o

.PHONY: clean, env

env:
	export HDF5_CC=mpicc
	export HDF5_CLINKER=mpicc

clean:
	rm -f data/Data.h5
	rm -f obj/*
	rm -f main
