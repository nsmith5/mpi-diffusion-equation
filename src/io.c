#include <mpi.h>
#include <hdf5.h>
#include <stdlib.h>
#include <unistd.h>     // access() to check if file exists
#include "state.h"
#include "io.h"

hid_t io_init (const char *filename)
{
    /*
     *  Create a file to save data to for this session
     */

    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;
    hid_t plist_id;     /* Property list id */
    hid_t file_id;      /* File id */

    plist_id = H5Pcreate (H5P_FILE_ACCESS);
    H5Pset_fapl_mpio (plist_id, comm, info);

    file_id = H5Fcreate (filename,
                         H5F_ACC_TRUNC,
                         H5P_DEFAULT,
                         plist_id);
    H5Pclose (plist_id);
    return file_id;
}

void io_finalize (hid_t file_id)
{
    H5Fclose (file_id);
    return;
}

void save_state (state* s, hid_t file_id)
{
    hid_t plist_id;             /* Property list id */
    hid_t dset_id, group_id;    /* dataset id and group id */
    hid_t filespace, memspace;  /* File and memory ids */
    hsize_t dimsf[2];           /* Data dimensions */
    hsize_t count[2];           /* Hyperslab stuff*/
    hsize_t offset[2];          /* Offset of starting points */
    herr_t status;

    /*
     * Dataspace specs
     */
    dimsf[0] = s->N;
    dimsf[1] = s->N;
    filespace = H5Screate_simple(2, dimsf, NULL);

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

    /*
     * Create Dataset
     */
    dset_id = H5Dcreate(group_id,
                        "Temperature",
                        H5T_NATIVE_DOUBLE,
                        filespace,
                        H5P_DEFAULT,
                        H5P_DEFAULT,
                        H5P_DEFAULT);
    H5Sclose (filespace);

    /*
     * Each process writes it's chunk of the temperature field to the
     * hyperslab in the file
     */

    count[0] = s->local_n0;
    count[1] = s->N;
    offset[0] = s->local_0_start;
    offset[1] = 0;
    memspace = H5Screate_simple(2, count, NULL);

    /*
     * Create your hyperslab
     */
    filespace = H5Dget_space (dset_id);
    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, count, NULL);

    /*
     * Create property list of collective dataset write.
     */
    plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);

    // Write data!
    status = H5Dwrite (dset_id,
                       H5T_NATIVE_DOUBLE,
                       memspace,
                       filespace,
                       plist_id,
                       s->T);

    // Close a bunch of stuff
    H5Gclose (group_id);
    H5Dclose (dset_id);
    H5Sclose (filespace);
    H5Sclose (memspace);
    H5Pclose (plist_id);

    MPI_Barrier (MPI_COMM_WORLD);

    return;
}
