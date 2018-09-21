Conda Build & Packaging
=======================

A set of smtk modules can be built and used in conda environments:

    smtk.attribute
    smtk.common
    smtk.io
    smtk.mesh
    smtk.model
    smtk.operation
    smtk.resource
    smtk.session.mesh
    smtk.session.polygon
    smtk.simulation
    smtk.view


Conda Setup for Developer Build
-------------------------------
As of 03-Oct-2018, developer builds from conda environments are not supported.
This is because the conda-build recipe was updated to build libMOAB and include
it in the smtk package (rather than implement a separate package).

However, it should be possible to use the build.sh script if you externally define
* SRC_DIR
* PREFIX
* CPU_COUNT

For devloper builds, these packages are needed in the conda environment.

    cmake
    make           (ninja should work too, but hasn't been tested)
    python=2.7     (python 3 might be OK, but hasn't been tested)
    gxx_linux-64   (or clangxx_osx-64 for macOS)
    boost
    eigen
    hdf5
    netcdf4
    nlohmann_json  (conda-forge)  (see note about installing this before moab)
    pybind11

Note that:

* python=2.7: Python version 3 might be OK, but hasn't been tested. Some source updates
  might also be needed for pybind11 compatibility.

For linux builds, there are environment and spec files in the "conda" directory.


CMake
-----
Run cmake and make/ninja from your conda environment. In previous versions, cmake variables
had to be set manually for conda builds. The current implementation checks for CONDA_PREFIX
in the environment, and if found, generates build files for the conda env.

When cmake is run, the output messages include a line starting with the text
"Using conda environment ...". Make sure the path in that message points to your
conda environment root folder.

The cmake scripts also write out out a number of python variables:

    PYTHON_EXECUTABLE
    PYTHON_INCLUDE_DIRS
    PYTHON_LIBRARIES

Check the cmake output to make sure these variables point to the correct place in your
conda environment.

If the python info is correct, you can proceed to use make or ninja to build the modules.


CTest
-----
The conda build generates 100+ tests, 41 of which use python. By default, tests are not enabled,
in order to avoid some build problems with cxx tests. To run python tests:

* Use cmake or ccmake to set SMTK_ENABLE_TESTING to ON.
* You don't have to rebuild; the python tests are enabled by the cmake generator.
* Run "ctest -R Py".
* Before building or packaging, be sure to reset SMTK_ENABLE_TESTING back to OFF.

To run the python tests:

    ctest -R Py

As of 27-Sep-2018, 3 python tests are not run, and 3 other tests fail with hard crashes:

      143 - polygonForceCreateFacePy (Skipped)
        7 - QuerySMTKPython (Not Run)
       58 - testImportPythonOperation (Not Run)
      101 - cellFieldPy (Child aborted)
      102 - meshMetricsPy (Child aborted)
      152 - loadExodusFilePy (Child aborted)


Conda Packaging
---------------
A meta.yaml file is including in the conda directory for packaging. It is not hard-coded to python 2.7, so pass that in when running conda build, for example:

    conda build --python=2.7 -c defaults -c conda-forge .../smtk/conda/meta.yaml

Note that you do not need to run conda-build from a conda environment.
On linux systems, the resulting package is a 5MB bzip2 file with a name like
smtk-3.0.0conda-py27h6bb024c_0.tar.bz2.


Other Notes
-----------
In the polygon session, the Import and ExtractContour classes are not included,
because they are dependent on VTK, which is not included in smtk-conda.
