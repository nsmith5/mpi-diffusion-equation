#include <mpi.h>
#include <hdf5.h>
#include <stdlib.h>
#include "state.h"
#include "io.h"

void save_state (state* s, char *filename)
{
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    hid_t plist_id;             /* Property list id */
    hid_t file_id, dset_id;     /* File and dataset ids */
    hid_t filespace, memspace;  /* File and memory ids */
    hsize_t dimsf[2];           /* Data dimensions */
    hsize_t count[2];           /* Hyperslab stuff*/
    hsize_t offset[2];          /* Offset of starting points */
    herr_t status;

    /*
     * Set up file access property list with parallel I/O access
     */
    plist_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_mpio(plist_id, comm, info);

    /*
     * Create a new file
     */
    file_id = H5Fcreate(filename,
                        H5F_ACC_TRUNC,
                        H5P_DEFAULT,
                        plist_id);
    H5Pclose(plist_id);

    /*
     * Dataspace specs
     */
    dimsf[0] = s->N;
    dimsf[1] = s->N;
    filespace = H5Screate_simple(2, dimsf, NULL);

    /*
     * Create Dataset
     */
    dset_id = H5Dcreate(file_id,
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
    status = H5Dwrite(dset_id, H5T_NATIVE_DOUBLE, memspace, filespace, plist_id, s->T);

    // Close a bunch of stuff
    H5Dclose(dset_id);
    H5Sclose(filespace);
    H5Sclose(memspace);
    H5Pclose(plist_id);
    H5Fclose(file_id);

    MPI_Barrier (MPI_COMM_WORLD);

    return;
}
