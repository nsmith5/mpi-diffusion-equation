#include <stdlib.h>
#include <math.h>
#include <fftw3.h>
#include <omp.h>

#include "state.h"

#define PI 2*acos(0)

double k_squared(int    i,
                 int    j,
                 int    N,
                 double dx)
{
    /*
     *  Compute the wavenumber at [i,j] for N x N system
     */
    double L = N*dx;
    double kx2 = i < (N>>1) + 1 ? pow (2*PI*i/L, 2) : pow (2*PI*(N-i)/L, 2);
    double ky2 = j < (N>>1) + 1 ? pow (2*PI*j/L, 2) : pow (2*PI*(N-j)/L, 2);

    return sqrt(kx2 + ky2);
}

state* create_state (int    N,
                     double dx,
                     double dt,
                     double D)
{
	/*
     *  Try to allocate new state
     */
    state* s = malloc (sizeof (state));
    if (s == NULL) return NULL;

    /*
     *  Try to allocate arrays
     */
    s->T = fftw_malloc (N * N * sizeof (double));
    s->G = fftw_malloc (N * (N/2 + 1) * sizeof (double));
    s->fT = fftw_malloc (N * (N/2 + 1) * sizeof (fftw_complex));

    if (s->T == NULL || s->fT == NULL || s->G == NULL)
    {
        // Bail if allocation failed and return null state
        free (s->T);
        free (s->fT);
        free (s->G);
        return NULL;
    }

    double kk;
    for (int i = 0; i < N; i++)
    {
      for (int j = 0; j < N/2 + 1; j++)
        {
          kk = k_squared (i, j, N, dx);
          s->G[i*(N/2+1)+j] = exp(-D*kk*dt)/N/N;
        }
    }

    /*
     *  Make Fourier transform plan
     */
    fftw_plan_with_nthreads (omp_get_max_threads ());
    s->fft_plan = fftw_plan_dft_r2c_2d (N, N, s->T, s->fT, FFTW_MEASURE);
    s->ifft_plan = fftw_plan_dft_c2r_2d (N, N, s->fT, s->T, FFTW_MEASURE);

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

void make_square (state  *s,
                  double  h)
{
    // Initialize state is square of height `h`
    int N = s->N;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (j >= N>>2 && j <= 3*(N>>2) && i >= N>>2 && i <= 3*(N>>2))
                s->T[i*N + j] = h;
            else s->T[i*N + j] = 0.0;
        }
    }
    return;
}

void make_const (state  *s,
                 double  h)
{
    // Initialize state as constant field of value `h`
    for (int i = 0; i < s->N; ++i)
    {
        for (int j = 0; j < s->N; ++j)
        {
            s->T[i*s->N + j] = h;
        }
    }

    return;
}

