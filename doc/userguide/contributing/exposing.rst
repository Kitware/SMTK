Exposing SMTK for use in external projects
==========================================

SMTK generates a file named :file:`SMTKConfig.cmake` that allows other projects to find and use SMTK.
This file is installed to :file:`${CMAKE_INSTALL_PREFIX}/lib/cmake/SMTK/`.
External projects can add SMTK with

.. code:: cmake

    find_package(SMTK)

Then, when building external projects, set CMake's :cmake:`SMTK_DIR` to the directory containing :file:`SMTKConfig.cmake`.
Note that you may point external projects to the top level of an SMTK build directory or
an install tree's :file:`lib/cmake/SMTK` directory; both contain an :file:`SMTKConfig.cmake` file
suitable for use by external projects.
The former is suitable for development, since you may be modifying both SMTK and a project
that depends on it — having to re-install SMTK after each change becomes tedious.
The latter is suitable for creating packages and distributing software.

If you add a new dependency to SMTK, :file:`CMake/smtkConfig.cmake.in` (which is used to create
:file:`SMTKConfig.cmake`) should be configured to find the dependent package so that consumers
of SMTK have access to it without additional work.

If you add a new option to SMTK, it should be exposed in :file:`CMake/Options.h.in`.
