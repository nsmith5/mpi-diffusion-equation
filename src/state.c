#include <stdlib.h>
#include <math.h>
#include <fftw3-mpi.h>
#include <fftw3.h>
#include "state.h"

#define PI 2*acos(0)

extern int threads_ok;

double k_squared(int i,
                 int j,
                 int N,
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

state* create_state (int N,
                     double dx,
                     double dt,
                     double D)
{
        /*
         *  Size of local array allocation
         */
        ptrdiff_t local_alloc;

        /*
         *  Try to allocate new state
         */
        state* s = malloc (sizeof (state));
        if (s == NULL) return NULL;

        /*
         *  Try to allocate temperature pointers
         */
        local_alloc = fftw_mpi_local_size_2d (N, N/2 + 1, MPI_COMM_WORLD,
                                              &s->local_n0, &s->local_0_start);

        s->T = fftw_alloc_real (2 * local_alloc);
        s->fT = fftw_alloc_complex (local_alloc);
        s->G = fftw_alloc_real (local_alloc);

        if (s->T == NULL || s->fT == NULL || s->G == NULL)
        {
                // Bail if allocation failed return null state
                free (s->T);
                free (s->fT);
                free (s->G);
                return NULL;
        }

        double kk;
        for (int i = 0; i < s->local_n0; i++)
        {
            for (int j = 0; j < N/2 + 1; j++)
            {
                kk = k_squared (i + s->local_0_start, j, N, dx);
                s->G[i*(N/2+1)+j] = exp(-D*kk*dt)/N/N;
            }
        }

        /*
         *  Make Fourier transform plan
         */
        s->fft_plan = fftw_mpi_plan_dft_r2c_2d (N, N, s->T, s->fT, MPI_COMM_WORLD,
                                                FFTW_MEASURE);

        s->ifft_plan = fftw_mpi_plan_dft_c2r_2d (N, N, s->fT, s->T, MPI_COMM_WORLD,
                                                 FFTW_MEASURE);

        // Initialize remaining parameters
        s->t = 0.0;
        s->N = N;
        s->dx = dx;
        s->dt = dt;
        s->D = D;

        MPI_Barrier(MPI_COMM_WORLD);
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
                  double h)
{
        // Initialize state is square of height `h`
        int I;
        int N = s->N;
        for (int i = 0; i < s->local_n0; i++)
        {
                for (int j = 0; j < N; j++)
                {
                        I = i + s->local_0_start;
                        if (j >= N>>2 && j <= 3*(N>>2) && I >= N>>2 && I <= 3*(N>>2))
                                s->T[i*2*(N/2 + 1) + j] = h;
                        else s->T[i*2*(N/2 + 1) + j] = 0.0;
                }
        }

        return;
}

void make_const (state  *s,
                 double h)
{
        // Initialize state as constant field of value `h`
        for (int i = 0; i < s->local_n0; ++i)
        {
                for (int j = 0; j < s->N; ++j)
                {
                        s->T[i*2*(s->N/2 + 1) + j] = h;
                }
        }

        return;
}
