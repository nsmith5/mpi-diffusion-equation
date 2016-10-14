/*
 * This file is part of Diffusion Equation MPI
 * Nathan Smith (c) 2016
 *
 * This file impliments I/0 using HDF5
 */


#include <mpi.h>
#include <hdf5.h>
#include <stdlib.h>
#include <unistd.h>     // access() to check if file exists
#include <stdbool.h>
#include <assert.h>

#include "error.h"
#include "state.h"
#include "io.h"

#define FAIL -1
#define MSG(x) if (io_verbose) printf("%s\n", x)

const int io_verbose = true;

hid_t io_init (const char *filename)
{
    /*
     *  Create a file to save data to for this session
     */
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;
    hid_t plist_id;     /* Property list id */
    hid_t file_id;      /* File id */
    herr_t err;         /* Error status */

    plist_id = H5Pcreate (H5P_FILE_ACCESS);
    assert (plist_id != FAIL);
    MSG("File Access granted");

    H5Pset_fapl_mpio (plist_id, comm, info);

    file_id = H5Fcreate (filename,
                         H5F_ACC_TRUNC,
                         H5P_DEFAULT,
                         plist_id);

    assert (file_id != FAIL);
    MSG("File created");

    err = H5Pclose (plist_id);
    assert (err != FAIL);

    MSG("HDF5 I/O Initialized");
    return file_id;
}

void io_finalize (hid_t file_id)
{
    /*
     *  Close file
     */
    herr_t err;
    err = H5Fclose (file_id);
    assert (err != FAIL);
    MSG("HDF5 I/O Finalized");
    return;
}

void save_state (state* s, hid_t file_id)
{
    /*
     *  Save state data to file
     */
    hid_t plist_id;             /* Property list id */
    hid_t dset_id, group_id;    /* dataset id and group id */
    hid_t filespace, memspace;  /* File and memory ids */
    hsize_t dimsf[2];           /* Data dimensions */
    hsize_t count[2];           /* Hyperslab stuff*/
    hsize_t offset[2];          /* Offset of starting points */
    herr_t status;

    /*
     * Dataspace specs
     *
     *  - space is [N x N] double precision
     */
    dimsf[0] = s->N;
    dimsf[1] = s->N;
    filespace = H5Screate_simple(2, dimsf, NULL);
    assert (filespace != FAIL);

    /*
     * Make Group from simulation time `t`
     */
    char groupname[50];
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
                        filespace,
                        H5P_DEFAULT,
                        H5P_DEFAULT,
                        H5P_DEFAULT);
    assert (dset_id != FAIL);
    status = H5Sclose (filespace);
    assert (status != FAIL);

    /*
     * Each process writes it's chunk of the temperature field to the
     * hyperslab in the file
     */
    for (int row = 0; row < s->local_n0; row++)
    {
      count[0] = 1;         // Only write one row
      count[1] = s->N;
      offset[0] = s->local_0_start + row;
      offset[1] = 0;

      memspace = H5Screate_simple (2, count, NULL);
      assert (memspace != FAIL);

      filespace = H5Dget_space (dset_id);
      assert (filespace != FAIL);

      status = H5Sselect_hyperslab (filespace, H5S_SELECT_SET, offset, NULL, count, NULL);
      assert (status != FAIL);

      plist_id = H5Pcreate (H5P_DATASET_XFER);
      assert (status != FAIL);

      H5Pset_dxpl_mpio (plist_id, H5FD_MPIO_COLLECTIVE);

      status = H5Dwrite (dset_id,
                         H5T_NATIVE_DOUBLE,
                         memspace,
                         filespace,
                         plist_id,
                         s->T + row*2*(s->N/2 + 1));
      if (status == FAIL) my_error ("Error on hdf5 write");
    }

    if (s->local_n0 > 0)
    {
      status = H5Pclose (plist_id);
      assert (status != FAIL);
      status = H5Sclose (memspace);
      assert (status != FAIL);
    }

    // Close a bunch of stuff
    status = H5Gclose (group_id);
    assert (status != FAIL);
    status = H5Dclose (dset_id);
    assert (status != FAIL);
    status = H5Sclose (filespace);
    assert (status != FAIL);
    return;
}
