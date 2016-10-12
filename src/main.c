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
    hid_t file_id = io_init ("data/Data.h5");

    int N = 1024;
    double dx = 0.1;
    double dt = 0.1;
    double D = 1.0;
    state* s = create_state (N, dx, dt, D);
    make_square (s, 1.0);

    for (int i = 0; i < 100; i++)
    {
      step (s);
      save_state (s, file_id);
    }

    destroy_state (s);
    io_finalize (file_id);
    finalize ();
    return 0;
}
