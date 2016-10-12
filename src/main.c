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

    state* s = create_state (24, 0.1, 0.01, 1.0);
    make_const (s, 1.0);

	fft (s);
	ifft (s);

	save_state (s, file_id);

    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Barrier(MPI_COMM_WORLD);

    for (int proc = 0; proc < size; proc++)
    {
        if (proc == rank)
        {
            for (int i = 0; i < s->local_n0; i++)
            {
                printf("\n");
                for (int j = 0; j < 24; j++)
                {
                    printf("%g ", s->T[i*26 + j]);
                }
            }
        }
    }

    printf("\n");

    destroy_state (s);
    io_finalize (file_id);
    finalize ();
    return 0;
}
