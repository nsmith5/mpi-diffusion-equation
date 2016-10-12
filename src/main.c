#include <mpi.h>
#include <hdf5.h>
#include <fftw3-mpi.h>
#include <stdio.h>
#include "state.h"
#include "dynamics.h"
#include "io.h"

void init (int    argc,
           char **argv)
{
    MPI_Init (&argc, &argv);
    fftw_mpi_init ();
    return;
}

void finalize (void)
{
    fftw_mpi_cleanup ();
    MPI_Finalize ();
    return;
}

int main (int argc, char **argv)
{
    init (argc, argv);

    state* s = create_state (20, 0.1, 0.2, 1.0);
    make_square (s, 1.0);
    save_state (s, "filename.h5");
    destroy_state (s);

    finalize ();
    return 0;
}
