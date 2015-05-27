******************
First steps in C++
******************

.. contents::
.. highlight:: c++
.. role:: cxx(code)
   :language: c++

To get started with SMTK, let's create a project that does the
bare minimum; we'll just print the SMTK version number and exit.

Including SMTK headers and calling methods
==========================================

The code to do this just calls a static method on :smtk:`smtk::common::Version`:

.. literalinclude:: print_version.cxx
   :start-after: // ++ 1 ++
   :end-before: // -- 1 --
   :linenos:

All of SMTK's headers are prefixed by "smtk," and because the
version number is useful across all of SMTK and not just particular
subsystems, it is put in the "common" directory.
Usually — but not always — the subdirectories indicate the C++
namespace that a class lives in.
Exceptions to this rule are classes in the extension directory,
which are grouped separately because they appear in other libraries
that have additional dependencies.


Compiling the example
=====================

To compile the program above, we need to link to SMTK's main library, named smtkCore.
The following :file:`CMakeLists.txt` will set up a project for us:

.. literalinclude:: CMakeLists.txt
   :language: cmake
   :start-after: # ++ 1 ++
   :end-before: # -- 1 --
   :linenos:

The :samp:`find_package` directive tells CMake to find SMTK on your system
and import its settings.
SMTK provides settings in a file named :file:`SMTKConfig.cmake` and it is
usually stored in :file:`/usr/local/lib/cmake/SMTK` on Unix and Mac OS X
systems or :file:`C:\\SMTK\\lib\\cmake\\SMTK` on Windows.
When CMake asks for :file:`SMTK_DIR`, you should provide it
with the directory containing :file:`SMTKConfig.cmake`.

Then, the :samp:`target_link_libraries` directive tells CMake not only
to link to smtkCore, but also to add compiler directives to all source
code in the executable specifying the location of header files.
This directive also adds any transitive dependencies of smtkCore to
the :file:`print_version` program.
