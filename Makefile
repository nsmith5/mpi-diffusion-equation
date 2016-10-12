CC = h5pcc
CFLAGS = -I./include -Wall
LIBS = -lfftw3_mpi -lfftw3 -lm

main: obj/state.o obj/main.o obj/io.o obj/dynamics.o
	$(CC) obj/* $(CFLAGS) $(LIBS) -o main

obj/main.o:
	$(CC) -c src/main.c $(CFLAGS) $(LIBS) -o obj/main.o

obj/state.o:
	$(CC) -c src/state.c $(CFLAGS) $(LIBS) -o obj/state.o

obj/io.o:
	$(CC) -c src/io.c $(CFLAGS) $(LIBS) -o obj/io.o

obj/dynamics.o:
	$(CC) -c src/dynamics.c $(CFLAGS) $(LIBS) -o obj/dynamics.o

.PHONY: clean, env

env:
	export HDF5_CC=mpicc
	export HDF5_CLINKER=mpicc

clean:
	rm -f obj/*
	rm -f main
