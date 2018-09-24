Building in Conda Environment
=============================

A set of smtk modules can be built from a conda environment:

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


Conda Setup
-----------
These packages are needed in the conda environment.

    cmake (install this first?)
    make  (install second?)
    python=2.7
    gcc_linux-64
    gxx_linux-64
    boost
    moab           (conda-forge)  (install this *AFTER* nlohmann_json)
    nlohmann_json  (conda-forge)
    pybind11

Note about moab: Strange as this seems, it appears that installing nlohmann_json blows
away the cmake config file for moab in the conda environment
(/lib/cmake/MOAB/MOABConfig.cmake). When that happens, the only known workaround is to
uninstall moab then install it a second time.

For linux builds, there is a spec file in the "conda" directory: conda-spec-file-linux64.txt.

**Use the conda environment for all cmake and build steps.**


CMake
-----
When you run CMake the first time, set the CONDA_BUILD option, for example:

    cmake  -DCONDA_BUILD=ON  -DCMAKE_TOOLCHAIN_FILE=${path-to-smtk-source}/conda/CondaToolchain.cmake  ${path-to-smtk-source}


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
The conda build generates 100+ tests, 41 of which use python. To run the python tests:

    ctest -R Py

As of 22-Sep-2018, 1 python test is skipped and 3 python tests fail with hard crashes:

    The following tests did not run:
      143 - polygonForceCreateFacePy (Skipped)

    The following tests FAILED:
      101 - cellFieldPy (Child aborted)
      102 - meshMetricsPy (Child aborted)
      152 - loadExodusFilePy (Child aborted)


Other Notes
-----------
As implied by the conda package list above, the conda build is currently limited to python 2.7.

In the polygon session, the Import and ExtractContour classes are not included,
because they are dependent on VTK, which is not included in smtk-conda.
