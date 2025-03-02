# Simulation Modeling Tool Kit (SMTK)

SMTK is a library which provides a way to specify attributes
(such as initial and boundary conditions, but also things
like mesh sizing functions) that describe a physical simulation
on a geometric domain, as well as high-level access to model
geometry (through various solid model and mesh tools) for the
purpose of linking attributes to geometric models of the
simulation domain.

## Documentation

The SMTK documentation is at [Read The Docs](http://smtk.readthedocs.org/en/latest/index.html).
What's below is a quick guide to building SMTK.

## Building SMTK

In order to build SMTK you must have

+ A modern C++ compiler  that supports C++11 features
 + gcc 4.8 or newer
 + Xcode 7.1 or newer
 + Visual Studio 2013 64 bit or newer
+ [CMake](http://cmake.org) 3.5 or newer


We recommend using [Ninja](http://martine.github.io/ninja/) for fast builds.

These components are provided by the CMB superbuild, and are mostly optional:

+ [Boost](http://boost.org) 1.60.0 or newer (required)
+ [OpenCascade](http://opencascade.org/) 7.4.0 or newer for importing CAD models;
+ [Python](http://python.org) version 2.7.3 or newer and
  [Pybind11](http://github.com/pybind/pybind11), for Python wrappings of
  SMTK's C++ classes;
+ [Qt](http://qt-project.org) version 5.12 or newer,
  for widgets to interact with attributes and models;
+ [Remus](https://github.com/robertmaynard/Remus) from the master branch,
  for running SMTK modelers in remote processes;
  and
+ [ParaView](http://paraview.org) version 4.3 or newer,
  or [VTK](http://VTK.org) version 6.2 or newer,
  for graphical presentation and selection of geometric models.

If you want to build the documentation you will need

+ [Doxygen](http://doxygen.org/) version 1.8 or newer,
+ [Graphviz](http://graphviz.org/) version 2 or newer,
+ [Sphinx](http://sphinx-doc.org/) version 1.2 or newer,
+ [Pygments](http://pygments.org/) version 1.6 or newer,
+ [breathe](http://breathe.readthedocs.org/en/latest/) version 3.1 or newer,
+ [doxylink](https://pypi.python.org/pypi/sphinxcontrib-doxylink) version 1.3 or newer,
+ [actdiag](https://pypi.python.org/pypi/sphinxcontrib-actdiag) version 1.0 or newer, and
+ optionally [sphinx_rtd_theme](https://pypi.org/project/sphinx-rtd-theme/) for a much-improved appearance.
+ Ubuntu:
  + `sudo apt-get install doxygen graphviz`
  + `pip install -U Sphinx Pygments breathe sphinxcontrib-doxylink sphinxcontrib-actdiag sphinx-rtd-theme`

In order to obtain the testing data used by SMTK's data directory, you will
need to use [git-lfs](https://git-lfs.github.com/). Once you have it available
in your `PATH` environment variable, you can run:

  * `git lfs install --local`
  * `git lfs fetch`
  * `git lfs checkout`

Rather than try to compile all the dependencies separately, we recommend using
the [CMB superbuild](https://gitlab.kitware.com/cmb/cmb-superbuild). It can
also be used to compile SMTK itself, and ModelBuilder, the canonical application
that uses SMTK. Most developers of SMTK turn on "developer-mode" for SMTK in
the superbuild, which prepares a cmake initialization file for an external
SMTK build.

Once you have prepared all of the dependencies, it is time to
create a build directory (again, *outside* the SMTK source directory
containing this ReadMe file) and run CMake.
These instructions will assume you have the SMTK source in a
directory named `/source/SMTK`, the SMTK test-data in `/data/SMTK`,
that you wish to build in a directory named `/build/SMTK`, and will
install SMTK into `/install/SMTK`.
To begin:

    mkdir /build/SMTK
    cd /build/SMTK
    # If you do not have Ninja and do not have the SMTK test data:
    cmake /source/SMTK
    # or, if you have Ninja and the SMTK test data:
    cmake -G Ninja -DSMTK_DATA_DIR:PATH=/data/SMTK /source/SMTK
    # or, if you used the superbuild:
    cmake -G Ninja -C /build/cmb-superbuild/smtk-developer-config.cmake -DSMTK_ENABLE_TESTING:BOOL=ON ../src

At this point, without the superbuild, CMake will likely complain that you are
missing Boost. It will also default to not build python wrappings, Qt, VTK,
ParaView, or OpenCASCADE functionality since those require optional
dependencies. To change these defaults and specify where Boost is located, you
may do any of the following:

+ manually edit the newly-created `/build/SMTK/CMakeCache.txt` file,
+ run the Qt `cmake-gui` command (assuming it is present in your build of CMake), or
+ run the text-based `ccmake` command (assuming it is present in your build of CMake).

Once you have updated all of the required settings, reconfigure (by re-running
`cmake`, pressing the "Configure" button, or pressing the `c` key, respectively --
depending on which method above you use to edit CMake settings). Once CMake is
in a good state, it will generate project files (either Makefiles, Ninja build
files, or an IDE project) either automatically (if you manually re-run `cmake`)
or at your request (when running `cmake-gui` or `ccmake`).
You may then build and install SMTK using the generated project files.

The documentation is not built by default, but
CMake exposes targets named `doc` and `doc-userguide`
that you can build.
The `doc` target builds the doxygen reference documentation
which appears in `/build/SMTK/doc/reference/smtk/html/index.xhtml`
while the `doc-userguide` target builds the user's guide and
tutorials which appear in `/build/SMTK/doc/user/html/index.html`.
