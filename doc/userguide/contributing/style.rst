Coding Conventions
==================

The SMTK source responsibility contains files that following 3 distinctive coding styles:

* SMTK Coding Style which should be used for all files except for those in the smtk/extension directory
* For any files found under the ``smtk/extension/vtk`` directory, the VTK Coding Conventions should be followed and can be found `here <https://docs.vtk.org/en/latest/developers_guide/coding_conventions.html#>`_.  The reason for this is to facilitate migration of VTK functionality developed for SMTK into VTK if the opportunity arises.
* For any files found under the ``smtk/extension/paraview`` directory that start with vtk should also follow the VTK Coding Conventions.
* For any files found under the ``smtk/extension/paraview`` directory that start with pq can follow the QT or SMTK Coding Conventions.  The reason is that the two conventions are compatible with each other though the SMTK style is a bit more extensive.

SMTK Coding Style
-----------------

The coding style is based on clang-format (which is defined by the .clang-format `file <https://gitlab.kitware.com/cmb/smtk/-/blob/master/.clang-format?ref_type=heads>`_ in the top directory).  SMTK CI software process is driven by this and will provide an option to reformat your code in cases where this base style is violated. T In addition to this base style, all files following this style must adhere to the following:

* No tabs or trailing whitespace are allowed.
* All files should end with a single newline (i.e., it is an error to have a source file without a newline or having multiple newlines at its end).
* Class names should be camel case, starting with an uppercase.
* Class member variables should start with ``m_`` or ``s_`` for per-instance or class-static variables, respectively.
* Class methods should be camel case starting with a lowercase character (except acronyms which should be all-uppercase).
* Use shared pointers and a static :cxx:`create()` method for classes that own significant storage or must be passed by
  reference to their superclass.
* In terms of setting and getting a property/member variable of a class, the following conventions are used

  * setFoo(...) - sets a member variable *m_foo*
  * foo() const - returns the value of member variable *m_foo*
* SMTK does use namespaces and classes added to the repository should be embedded in the namespace that is appropriate.  Typically the namespace reflects the directory where the class is located. Some examples are:

  * smtk::attribute - for all Attribute Resource related classes and should found under the ``smtk/attribute`` directory
  * smtk::common - for all general purpose classes and should found under the ``smtk/common`` directory
  * smtk::extension - for all non-core classes - this would include extensions to QT, VTK, and ParaView and should found under the ``smtk/extension`` directory
  * smtk::io - for all I/O classes and should found under the ``smtk/io`` directory

All header files must be guarded and the guard name should include the namespace of the class for example smtk/attribute/Attribute.h uses smtk_attribute_Attribute_h as the guard name.

All .h and .cxx files must also include the following boiler plate:

.. code-block:: cpp

    //=========================================================================
    //  Copyright (c) Kitware, Inc.
    //  All rights reserved.
    //  See LICENSE.txt for details.
    //
    //  This software is distributed WITHOUT ANY WARRANTY; without even
    //  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
    //  PURPOSE.  See the above copyright notice for more information.
    //=========================================================================

This is required in order to pass the CopyrightStatement test.
