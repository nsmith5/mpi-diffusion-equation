#include <mpi.h>
#include <hdf5.h>
#include <fftw3-mpi.h>
#include <stdio.h>
#include <unistd.h>		// access () for checking file existance
#include <stdbool.h>
#include <omp.h>

#include "error.h"
#include "state.h"
#include "dynamics.h"
#include "io.h"

#define MSG(x) if (verbose) printf("%s\n", x)

const int verbose = true;
int threads_ok;
void init (int argc, char **argv);
void finalize (void);

int main (int argc, char **argv)
{
    int N = 512;
    double dx = 0.1;
    double dt = 0.1;
    double D = 1.0;

    init (argc, argv);
    hid_t file_id = io_init ("data/Data.h5");
    state* s = create_state (N, dx, dt, D);
    make_square (s, 1.0);

    double t1 = MPI_Wtime();
    for (int i = 0; i < 100; i++)
    {
      step (s);
      save_state (s, file_id);
    }
    double t2 = MPI_Wtime();

	int rank;
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	if (rank == 0)
    	printf("Elapsed time for loop is %f\n", t2-t1);

    destroy_state (s);
    io_finalize (file_id);
    MPI_Barrier (MPI_COMM_WORLD);
    finalize ();
    return 0;
}

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
  MSG("FFTW Initialized");

  // Import wisdom from file and broadcast
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0 && access ("data/plans.wisdom", F_OK) != -1)
  {
    int err = fftw_import_wisdom_from_filename ("data/plans.wisdom");
    if (err == 0) my_error("Importing FFTW wisdom failed!");
    MSG("FFTW Loaded Wisdom from file");
  }
  fftw_mpi_broadcast_wisdom (MPI_COMM_WORLD);
  MSG("FFTW Broadcasted Wisdom");
  MPI_Barrier (MPI_COMM_WORLD);
  return;
}

void finalize (void)
{
  // Gather wisdom from procs and save to file
  fftw_mpi_gather_wisdom (MPI_COMM_WORLD);
  int rank;
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  if (rank == 0)
  {
    int err = fftw_export_wisdom_to_filename ("data/plans.wisdom");
    if (err == 0)
      {
        remove ("data/plans.wisdom");
        my_error ("Failed to correctly export FFTW wisdom");
      }
  }

  MPI_Barrier (MPI_COMM_WORLD);
  // Clean up threads, fftw and finalize MPI runtime
  fftw_cleanup_threads ();
  fftw_mpi_cleanup ();
  MPI_Finalize ();
  return;
}
