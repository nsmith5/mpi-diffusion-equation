#include <fftw3.h>
typedef struct
{
	double* T;			// Temperature field
	fftw_complex* fT;	// Fourier Transform of the field
	double *G;			// The propagator

  	double t;			// Global time
  	double dt;			// Time step size
  	double dx;			// Grid spacing (square)
  	double D;			// Diffusion constant

  	ptrdiff_t N;				// N x N grid
	ptrdiff_t local_n0;			// local end point
  	ptrdiff_t local_0_start;	// local start point

  	fftw_plan fft_plan;		// Fourier transform plan
	fftw_plan ifft_plan;	// Inverse Fourier transform plan
}
state;

state* create_state (int    N,
                     double dx,
                     double dt,
                     double D);

void destroy_state (state* S);

void make_square (state *s, double h);

void make_const (state *s, double h);
