/*
 * This is a part of Diffusion Equation MPI
 * Nathan Smith (c) 2016
 *
 * This code implements error handling. If an error is detected somewhere
 * an error can be thrown by calling error("<Error string here>").
 */

#include <stdio.h>
#include <unistd.h>

#include "error.h"

void my_error (const char* error_string)
{
    printf ("-------------------------------------------------------\n");
    printf ("Error caught: %s\n", error_string);
    printf ("-------------------------------------------------------\n");
    exit (1);
}
