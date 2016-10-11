#include <stdlib.h>
#include <fftw3-mpi.h>
#include <fftw3.h>
#include "state.h"

state* create_state (int    N,
                     double dx,
                     double dt,
                     double D)
{
    ptrdiff_t local_alloc;      // Size of local array allocation

	// Try to allocate new state
    state* s = malloc (sizeof (state));
    if (s == NULL)
      return NULL;

    // Try to allocate temperature pointers
    local_alloc = fftw_mpi_local_size_2d(N,
                                         (N>>1) + 1,
                                         MPI_COMM_WORLD,
                                         &s->local_n0,
                                         &s->local_0_start);

    s->T = fftw_alloc_real (2 * local_alloc);
    s->fT = fftw_alloc_complex (local_alloc);

    if (s->T == NULL || s->fT == NULL)
    {
        // If allocation failed return null state
        free (s->T);
        free (s->fT);
        return NULL;
    }

    // Make Fourier transform plan
    s->fft_plan = fftw_mpi_plan_dft_r2c_2d(N,
                                           N,
                                           s->T,
                                           s->fT,
                                           MPI_COMM_WORLD,
                                           FFTW_MEASURE);

    s->ifft_plan = fftw_mpi_plan_dft_c2r_2d(N,
                                            N,
                                            s->fT,
                                            s->T,
                                            MPI_COMM_WORLD,
                                            FFTW_MEASURE);

    // Initialize remaining parameters
    s->t = 0.0;
    s->N = N;
  	s->dx = dx;
  	s->dt = dt;
  	s->D = D;

  	return s;
}

void destroy_state (state* s)
{
    // Free Memory of state
    if (s != NULL)
    {
        fftw_destroy_plan (s->fft_plan);
        fftw_destroy_plan (s->ifft_plan);
        free (s->fT);
        free (s->T);
  	    free (s);
    }
}
