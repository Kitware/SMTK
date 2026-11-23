.. _generating-pybind11-bindings:

Generating pybind11 bindings
============================

SMTK's pybind11_ bindings are generated via python scripts that use
pygccxml_ to construct a C++ header file corresponding to each header
file in SMTK, and a C++ source file for each namespace in
SMTK. Generally, python modules are a reflection of C++ namespaces
(which are generally contained within a subdirectory). The generated
pybind11 C++ files for each module are in the ``pybind`` subdirectory of
the module.

To generate a C++ header file for a new SMTK class, use
``[smtk-root-directory]/utilities/python/cpp_to_pybind11.py`` with the
appropriate arguments for the header file, project root directory,
include directories (e.g. ``-I [smtk-build-directory]
-I [smtk-root-directory]/thirdparty/cJSON path/to/vtk/include``) and
generated file prefix; the module's binding source file (also located
in the ``pybind`` subdirectory of the module) must then be updated to
call the functions defined in the generated header file. To generate
all of the C++ headers and the module C++ source file for a namespace
at once, use
``[smtk-root-directory]/utilities/python/generate_pybind11_module.py``
with the appropriate arguments for the module directory, project root
directory and include directories.

The generated bindings should be treated as a starting point for
customization to create a more *pythonic* interface.

.. _pybind11: http://pybind11.readthedocs.io
.. _pygccxml: http://pygccxml.readthedocs.io
