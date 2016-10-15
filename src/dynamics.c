#include <math.h>
#include <fftw3-mpi.h>
#include "state.h"
#include "dynamics.h"

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

    for (int i = 0; i < s->local_n0; i++)
    {
        for (int j = 0; j < (s->N>>1) + 1; j++)
        {
            s->fT[i*(s->N/2 + 1) + j][0] *= s->G[i*(s->N/2 + 1) + j];
            s->fT[i*(s->N/2 + 1) + j][1] *= s->G[i*(s->N/2 + 1) + j];
        }
    }

    MPI_Barrier (MPI_COMM_WORLD);

    fftw_mpi_execute_dft_c2r(s->ifft_plan, s->fT, s->T);

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
