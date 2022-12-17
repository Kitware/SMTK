Source code organization
========================

To a first approximation, SMTK's directory structure mirrors the namespaces used:
classes in the :cxx:`smtk::attribute` namespace are mostly found in the
:file:`smtk/attribute` directory.
Exceptions occur where classes that belong in a namespace depend on third-party libraries
that should not be linked to SMTK's core library.
For example, Qt widgets for attributes are in :file:`smtk/extensions/qt`, not in :file:`smtk/attribute`
because they depend on Qt, which is optional when building SMTK (so that, for instance,
solid modeling kernels like Cubit may be supported without issue).

With that in mind:

* smtk — this directory contains all of the source code for SMTK libraries and tests

  * common — source for classes used throughout the smtkCore library
  * resource — :ref:`smtk-resource-sys` defines base resources (files) and components used elsewhere
  * attribute — source for :ref:`smtk-attribute-sys` in the smtkCore library
  * model — source for :ref:`smtk-model-sys` in the smtkCore library
  * mesh — source for :ref:`smtk-mesh-sys` in the smtkCore library
  * operation — :ref:`smtk-operation-sys` provides asynchronous operations that act on resources
  * simulation — aids to exporting simulation input decks in the smtkCore library
  * io — file and string I/O in the smtkCore library, a mix of XML and JSON
  * view — source for providing views (user presentations) of resources in the smtkCore library
  * session — source for additional libraries that session solid modeling kernels into SMTK
  * extensions — source for additional libraries that expose SMTK to other software

    * qt — widgets that allow presentation and editing of SMTK models and attributes
    * vtk — VTK_ sources for displaying tessellations of model faces, edges, and vertices
    * paraview - user interface components that embed SMTK into branded ParaView_ applications

* thirdparty

  * cJSON — used to serialize geometric model information
  * pugiXML — used to serialize attribute resources

* utilities — scripts to aid in the development of SMTK.
  * encode – a C++ utility used to encode a file's contents into a C++ function,
    a C++ string literal, or a Python block-quote in order to embed it into a
    plugin or module.

Inside :file:`smtk/`, subdirectories, there are :file:`testing/` directories that
hold :file:`python/` and :file:`cxx/` directories for Python and C++ tests, respectively.
These are discussed more in :ref:`smtk-testing-sys`.

.. _VTK: http://vtk.org/
.. _ParaView: http://paraview.org/
