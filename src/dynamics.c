#include <math.h>
#include <fftw3.h>
#include <omp.h>

#include "state.h"
#include "dynamics.h"

void normalize (state *s)
{
    for (int i = 0; i < s->N; i++)
        for (int j = 0; j < s->N; j++)
          s->T[i*s->N + j] /= s->N*s->N;
    return;
}


void step(state *s)
{
    /*
     * Time step the state forward by dt
     */
    fftw_execute_dft_r2c(s->fft_plan, s->T, s->fT);

    int N = s->N;

    #pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < (N>>1) + 1; j++)
        {
            s->fT[i*((N>>1) + 1) + j][0] *= s->G[i*((N>>1) + 1) + j];
            s->fT[i*((N>>1) + 1) + j][1] *= s->G[i*((N>>1) + 1) + j];
        }
    }

    fftw_execute_dft_c2r(s->ifft_plan, s->fT, s->T);

    s->t += s->dt;
    return;
}

void fft (state *s)
{
	fftw_execute_dft_r2c (s->fft_plan, s->T, s->fT);
	return;
}

void ifft (state *s)
{
	fftw_execute_dft_c2r (s->ifft_plan, s->fT, s->T);
	normalize (s);
	return;
}
