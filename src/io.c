#include <mpi.h>
#include <hdf5.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "io.h"

#define ind(i,j) (j)*(bx+2)+(i)

void writehdf5(const double *a, int iter, int n, int px, int py, int bx,
	       int by, int offx, int offy, MPI_Comm comm)
{
  hid_t file_id;      // File identifier
  hid_t plist_id;     // File access property list identifier
  hid_t cplist_id;    // Collective write property list
  hid_t filespace;    // Data space
  hid_t memspace;     // Memory space
  hid_t dset_id;      // Data set id
  hid_t attr_id;      // Attribute id
  hid_t attr_mem_id;  // Attribute memory space

  hsize_t data_dims[2];
  hsize_t offset[2], stride[2], blocks[2], counts[2];

  double *b;
  int i, j;

  /*
   * Initialize C interface.
   */
  H5open();

  /*
   * Set up file access property list for MPI-IO access
   */
  plist_id = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, comm, MPI_INFO_NULL);

  // Create an new file.
  file_id = H5Fcreate("checkpoint_tmp.h5", H5F_ACC_TRUNC,
		      H5P_DEFAULT, plist_id);

  // Create data set.
  data_dims[0] = n;
  data_dims[1] = n;
  filespace = H5Screate_simple(2, data_dims, NULL);
  dset_id = H5Dcreate(file_id, "checkpoint", H5T_NATIVE_DOUBLE, filespace,
		      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  // create local memory space
  data_dims[0] = by;
  data_dims[1] = bx;
  memspace = H5Screate_simple(2, data_dims, NULL);
  offset[0] = offy;
  offset[1] = offx;
  stride[0] = py-1;
  stride[1] = px-1;
  blocks[0] = by;
  blocks[1] = bx;
  counts[0] = 1;
  counts[1] = 1;

  H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, stride, counts, blocks);

  // Property list for collective write
  cplist_id = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(cplist_id, H5FD_MPIO_COLLECTIVE);

  // write dataset collectively
  b = malloc(bx * by * sizeof *b);
  for (j = 0; j < by; j++)
    for (i = 0; i < bx; i++)
      b[j*bx+i] = a[ind(i+1,j+1)];
  H5Dwrite(dset_id, H5T_NATIVE_DOUBLE, memspace, filespace, cplist_id, b);
  free(b);

  attr_id = H5Acreate(dset_id, "iter", H5T_NATIVE_INT, H5Screate(H5S_SCALAR),
		      H5P_DEFAULT, H5P_DEFAULT);
  H5Awrite(attr_id, H5T_NATIVE_INT, &iter );
  H5Aclose(attr_id);

  // Close the file.
  H5Sclose(filespace);
  H5Pclose(cplist_id);
  H5Dclose(dset_id);
  H5Fclose(file_id);
  H5Pclose(plist_id);
  H5close();

  //
  // Rename (atomic operation in UNIX/Linux)
  //
  if (offx == 0 && offy == 0) {
    rename("checkpoint_tmp.h5", "checkpoint.h5");
  }
}
