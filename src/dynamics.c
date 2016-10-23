#include <math.h>
#include <fftw3-mpi.h>
#include "state.h"
#include "dynamics.h"

void normalize (state *s)
{
        // Normalize the temperature field, FFTW's fft generate a factor of
        // Nx*Ny that needs to scaled out afterwards.
        int ij;
        for (int i = 0; i < s->local_n0; i++)
                for (int j = 0; j < s->N; j++)
                {
                        ij = i*2*((s->N>>1) + 1) + j;
                        s->T[ij] /= s->N*s->N;
                }

        return;
}


void step(state *s)
{
        //Time step the state forward by dt
        int ij;
        fftw_mpi_execute_dft_r2c(s->fft_plan, s->T, s->fT);

        #pragma omp parallel for
        for (int i = 0; i < s->local_n0; i++)
        {
                for (int j = 0; j < (s->N>>1) + 1; j++)
                {
                        ij = i*((s->N >> 1) + 1) + j;
                        s->fT[ij][0] *= s->G[ij];
                        s->fT[ij][1] *= s->G[ij];
                }
        }

        fftw_mpi_execute_dft_c2r(s->ifft_plan, s->fT, s->T);
        s->t += s->dt;
        return;
}

void fft (state *s)
{
        fftw_mpi_execute_dft_r2c (s->fft_plan, s->T, s->fT);
        return;
}

void ifft (state *s)
{
        fftw_mpi_execute_dft_c2r (s->ifft_plan, s->fT, s->T);
        normalize (s);
        return;
}
