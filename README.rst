MPI Diffusion Equation Simulator
================================

This repository hosts a high performance program to simulate the diffusion equation.

.. math::

	\frac{\partial c(x, t)}{\partial t} = D\nabla^2 c(x, t)

This repository is place to learn MPI programming methods including parallel I/O, parallel FFTW fourier transforms and other MPI algorithm design.

Installation
------------

To install make sure you have an MPI compiler installed as well as FFTW3 and
HDF5 libraries installed that were compiled with the same compiler. Once this
is done simply compile with

.. code:: bash
	>> make


 
