/*
 * Copyright (c) 2012 Torsten Hoefler. All rights reserved.
 *
 * Author(s): Torsten Hoefler <htor@illinois.edu>
 *
 */

#include <mpi.h>
#include <hdf5.h>
#include <stdio.h>
#include "state.h"

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);
	int r,p;
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_rank(comm, &r);
	MPI_Comm_size(comm, &p);

	state* s = state_create(10, 0.1, 0.1, 1.0);

    s->T[1] = 1.0;

	printf("I'm process %d and my state dx is %f\n", r, s->T[0]);


  	MPI_Barrier(comm);

    state_destroy(s);

    MPI_Finalize();
}
