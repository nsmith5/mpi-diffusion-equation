#include <mpi.h>
#include <hdf5.h>
#include <fftw3-mpi.h>
#include <stdio.h>
#include "state.h"

void init (int    argc,
           char **argv)
{
    MPI_Init (&argc, &argv);
    //fftw_mpi_init ();
    return;
}

void finalize (void)
{
    //fftw_mpi_cleanup ();
    MPI_Finalize ();
    return;
}

int main (int argc, char **argv)
{
    init (argc, argv);

	int r,p;
	MPI_Comm comm = MPI_COMM_WORLD;

    state* s = create_state(100, 0.1, 0.2, 1.0);

    destroy_state(s);
    finalize ();
    return 0;
}
