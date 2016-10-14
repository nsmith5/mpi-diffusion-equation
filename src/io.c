/*
 * This file is part of Diffusion Equation MPI
 * Nathan Smith (c) 2016
 *
 * This file impliments I/0 using HDF5
 */

#include <hdf5.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "error.h"
#include "state.h"
#include "io.h"

#define FAIL -1

hid_t io_init (const char *filename)
{
    /*
     *  Create a file to save data to for this session
     */
    hid_t file_id;      /* File id */

    file_id = H5Fcreate (filename,
                         H5F_ACC_TRUNC,
                         H5P_DEFAULT,
                         H5P_DEFAULT);

    assert (file_id != FAIL);
    return file_id;
}

void io_finalize (hid_t file_id)
{
    herr_t err;

    err = H5Fclose (file_id);   /* Attempt to close file */
    assert (err != FAIL);       /* Assert that the file was properly closed */
    return;
}

void save_state (state* s, hid_t file_id)
{
    hid_t dset_id, group_id;    /* dataset id and group id */
    hid_t dataspace;            /* Dataspace id */
    hsize_t dimsf[2];           /* Data dimensions */
    herr_t status;              /* Error status */
    char groupname[50];         /* Name for time step group */

    /*
     * Dataspace specs
     *
     *  - space is [N x N] double precision
     */
    dimsf[0] = s->N;
    dimsf[1] = s->N;
    dataspace = H5Screate_simple(2, dimsf, NULL);

    /*
     * Make Group from simulation time `t`
     */
    sprintf(groupname, "Time = %g", s->t);
    group_id = H5Gcreate (file_id,
                          groupname,
                          H5P_DEFAULT,
                          H5P_DEFAULT,
                          H5P_DEFAULT);
    assert (group_id != FAIL);

    // Create Dataset
    dset_id = H5Dcreate(group_id,
                        "Temperature",
                        H5T_NATIVE_DOUBLE,
                        dataspace,
                        H5P_DEFAULT,
                        H5P_DEFAULT,
                        H5P_DEFAULT);
    assert (status != FAIL);

    status = H5Dwrite (dset_id,
                       H5T_NATIVE_DOUBLE,
                       H5S_ALL,
                       dataspace,
                       H5P_DEFAULT,
                       s->T);

    // Close a bunch of stuff
    status = H5Gclose (group_id);
    status = H5Dclose (dset_id);
    status = H5Sclose (dataspace);
    return;
}
