#include <hdf5.h>
#include <fftw3.h>
#include <omp.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "error.h"
#include "state.h"
#include "dynamics.h"
#include "io.h"

#define FILENAME "data/Data.h5"


void init (int argc, char **argv);
void finalize (void);

int main (int argc, char **argv)
{
    struct timespec t_start, t_end;
    double wtime;
    int N = 8192;
    double dx = 0.1;
    double dt = 0.1;
    double D = 1.0;
    state *s = create_state (N, dx, dt, D);
    hid_t file_id = io_init (FILENAME);

    /*
     * Initialize FFTW
     */
    init (argc, argv);

    /*
     * Make a square initial condition
     */
    make_square (s, 1.0);

    /*
     * - Time Step the state
     * - Save each time step
     * - Measure the time for the whole loop
     */

    clock_gettime (CLOCK_REALTIME, &t_start);
    for (int i = 0; i < 10; i++)
    {
      step (s);
      //save_state (s, file_id);
      //printf("Now on step %d\n", i);
    }
    clock_gettime (CLOCK_REALTIME, &t_end);
    wtime = (double)(t_end.tv_sec + t_end.tv_nsec*1e-9) -
            (double)(t_start.tv_sec + t_start.tv_nsec*1e-9);

    /*
     * Print out the results of the time trial
     */
    printf("Elapsed time for loop is %f\n", wtime);

    /*
     * Do clean up of the system before exiting
     */
    destroy_state (s);
    io_finalize (file_id);
    finalize ();
    return 0;
}

void init (int    argc,
           char **argv)
{
    /*
     *  FFTW initialize threads
     */
    fftw_init_threads ();
    fftw_plan_with_nthreads (omp_get_max_threads ());

    /*
     * Load FFTW Wisdom from file (save planning time)
     */
    if (access ("data/plans.wisdom", F_OK) != -1)
    {
        int err = fftw_import_wisdom_from_filename ("data/plans.wisdom");
        if (err == 0) my_error("Importing FFTW wisdom failed!");
    }
    return;
}

void finalize (void)
{
    /*
     * Export FFTW Wisdom to file (for next time)
     */
    int err = fftw_export_wisdom_to_filename ("data/plans.wisdom");
    if (err == 0)
    {
        remove ("data/plans.wisdom");
        my_error ("Failed to correctly export FFTW wisdom");
    }

    fftw_cleanup ();
    return;
}
