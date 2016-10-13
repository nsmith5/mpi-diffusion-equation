#include <mpi.h>
#include <hdf5.h>
#include <fftw3-mpi.h>
#include <stdio.h>
#include <omp.h>

#include "state.h"
#include "dynamics.h"
#include "io.h"

int threads_ok;

void init (int    argc,
           char **argv)
{
  // Check level of threading provided and initialize threads
  int provided;
  MPI_Init_thread (&argc, &argv, MPI_THREAD_FUNNELED, &provided);
  if (provided < MPI_THREAD_FUNNELED) MPI_Abort (MPI_COMM_WORLD, 1);
  threads_ok = provided;
  if (threads_ok) threads_ok = fftw_init_threads ();
  if (threads_ok) fftw_plan_with_nthreads (omp_get_max_threads ());

  // Initialize fftw
  fftw_mpi_init ();

  // Import wisdom from file and broadcast
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) fftw_import_wisdom_from_filename ("data/plans.wisdom");
  fftw_mpi_broadcast_wisdom (MPI_COMM_WORLD);

  return;
}

void finalize (void)
{
  // Gather wisdom from procs and save to file
  fftw_mpi_gather_wisdom (MPI_COMM_WORLD);
  int rank;
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  if (rank == 0) fftw_export_wisdom_to_filename ("data/plans.wisdom");

  // Clean up threads, fftw and finalize MPI runtime
  fftw_cleanup_threads ();
  fftw_mpi_cleanup ();
  MPI_Finalize ();
  return;
}

int main (int argc, char **argv)
{
    init (argc, argv);
    hid_t file_id = io_init ("data/Data.h5");

    int N = 4096;
    double dx = 0.1;
    double dt = 0.1;
    double D = 1.0;
    state* s = create_state (N, dx, dt, D);
    make_square (s, 1.0);

    double t1 = MPI_Wtime();
    for (int i = 0; i < 100; i++)
    {
      step (s);
      //save_state (s, file_id);
    }
    double t2 = MPI_Wtime();

    printf("Elapsed time for loop is %f\n", t2-t1);

    destroy_state (s);
    io_finalize (file_id);
    finalize ();
    return 0;
}
