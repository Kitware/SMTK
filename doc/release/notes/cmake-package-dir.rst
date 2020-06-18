CMake package directory
=======================

The CMake package directory for SMTK is now in a location that CMake searches
by default. This removes the need to do ``-Dsmtk_DIR`` and instead the install
prefix can be given in the ``CMAKE_PREFIX_PATH`` variable.
