typedef struct
{
	double* T;		// Temperature field
  	double dt;		// Time step size
  	double dx;		// Grid spacing (square)
	double D;		// Diffusion constant
	int N;			// N x N grid
}
state;

state* state_create(int    N,
                    double dx,
                    double dt,
                    double D);

void state_destroy(state* S);
