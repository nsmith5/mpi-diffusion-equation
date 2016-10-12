#include <math.h>
#include <fftw3-mpi.h>
#include "state.h"
#include "dynamics.h"

#define PI 2*acos(0)

double k(state *s,
         int    i,
         int    j)
{
    /*
     * Compute the wavenumber at index (i,j)
     */
    int N = s->N;
    double L = s->N*s->dx;

    double kx2 = i < (N>>1) + 1 ? pow (2*PI*i/L, 2) : pow (2*PI*(N-i)/L, 2);
    double ky2 = j < (N>>1) + 1 ? pow (2*PI*j/L, 2) : pow (2*PI*(N-j)/L, 2);

    return sqrt(kx2 + ky2);
}

void normalize (state *s)
{
    for (int i = 0; i < s->local_n0; i++)
        for (int j = 0; j < s-> N; j++)
          s->T[i*2*(s->N/2 + 1) + j] /= s->N*s->N;
    return;
}


void step(state *s)
{
    /*
     * Time step the state forward by dt
     */
    fftw_mpi_execute_dft_r2c(s->fft_plan, s->T, s->fT);
    MPI_Barrier(MPI_COMM_WORLD);

    double kk;
    for (int i = 0; i < s->local_n0; i++)
    {
        for (int j = 0; j < (s->N>>1) + 1; j++)
        {
            kk = k(s, i + s->local_0_start, j);
            s->fT[i*(1 + (s->N>>1)) + j][0] *= exp(-s->D*kk*kk*s->dt);
            s->fT[i*(1 + (s->N>>1)) + j][1] *= exp(-s->D*kk*kk*s->dt);
        }
    }

    fftw_mpi_execute_dft_c2r(s->ifft_plan, s->fT, s->T);
    normalize(s);

    s->t += s->dt;
    MPI_Barrier(MPI_COMM_WORLD);

    return;
}

void fft (state *s)
{
	fftw_mpi_execute_dft_r2c (s->fft_plan, s->T, s->fT);
	MPI_Barrier (MPI_COMM_WORLD);
	return;
}

void ifft (state *s)
{
	fftw_mpi_execute_dft_c2r (s->ifft_plan, s->fT, s->T);
	normalize (s);
	MPI_Barrier (MPI_COMM_WORLD);
	return;
}
