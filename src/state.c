#include <stdlib.h>	// malloc
#include "state.h"

state* state_create (int    N,
                     double dx,
                     double dt,
                     double D)
{
	// Try to allocate new state
    state* s = malloc (sizeof (state));
    if (s == NULL)
      return NULL;

    // Try to allocate temperature pointer
    s->T = malloc (N * N * sizeof (double));
    if (s->T == NULL)
    {
        free (s->T);
        return NULL;
    }

  	s->N = N;
  	s->dx = dx;
  	s->dt = dt;
  	s->D = D;

  	return s;
}

void state_destroy (state* s)
{
    if (s != NULL)
    {
        free (s->T);
  	    free (s);
    }
}
