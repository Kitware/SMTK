********************
Contributing to SMTK
********************

.. role:: cxx(code)
   :language: c++

.. contents::

The first step to contributing to SMTK is to obtain the source code and build it.
The top-level ReadMe.mkd file in the source code includes instructions for building SMTK.
The rest of this section discusses how the source and documentation are organized
and provides guidelines for how to match the SMTK style.

Source code organization
========================

To a first approximation, SMTK's directory structure mirrors the namespaces used:
classes in the :cxx:`smtk::attribute` namespace are mostly found in the
:file:`smtk/attribute` directory.
Exceptions occur where classes that belong in a namespace depend on third-party libraries
that should not be linked to SMTK's core library.
For example, Qt widgets for attributes are in :file:`smtk/Qt`, not in :file:`smtk/attribute`
because they depend on Qt, which is optional when building SMTK (so that, for instance,
solid modeling kernels like Cubit may be supported without issue).

With that in mind:

* smtk — this directory contains all of the source code for SMTK libraries and tests

  * attribute — source for :ref:`smtk-attribute-sys` in the SMTKCore library
  * model — source for :ref:`smtk-model-sys` in the SMTKCore library
  * bridges — source for additional libraries that bridge solid modeling kernels into SMTK
  * Qt — source for Qt-based widgets that allow presentation and editing of SMTK models and attributes
  * extensions

    * vtk — source for displaying tessellations of model faces, edges, and vertices using VTK_

* thirdparty

  * cJSON — used to serialize geometric model information
  * pugiXML — used to serialize attribute systems
  * sparsehash — an alternative to using :cxx:`std::map` to store maps from UUIDs to entity and property records in SMTK models.


Inside :file:`smtk/`, subdirectories, there are :file:`testing/` and :file:`pythonTesting` directories
to hold C++ and Python tests, respectively.

Code style
==========

* No tabs or trailing whitespace are allowed.
* Indent blocks by 2 spaces.
* Class names should be camel case, starting with an uppercase.
* Class member variables should start with :cxx:`m_` or :cxx:`s_` for per-instance or class-static variables, respectively.
* Class methods should be camel case starting with a lowercase character (except acronyms which should be all-uppercase).
* Use shared pointers and a static :cxx:`create()` method for classes that own significant storage or must be passed by
  reference to their superclass.

Using SMTK from another project
===============================

.. todo::

  SMTK does not currently export an SMTKConfig.cmake file like it should.

Extending SMTK
==============

See the tutorials for in-depth guides on how to extend SMTK
in certain obvious directions,

* Writing an attribute system template file to represent a solver's input format.
* Writing an exporter to support a new solver's input format.
* Adding a new solid-modeling operator
* Bridging SMTK to a new solid-modeling kernel

Documentation style
===================

There are two types of documentation in SMTK:
Doxygen_ documentation written as comments in C++ code and
Sphinx_ documentation written in reStructuredText_ files (and optionally Python documentation strings).
The former is used to create reference documentation; the latter is used for the user's guide and tutorials.

The following rules apply to writing documentation:

* Header files should contain the Doxygen documentation for the class as a whole plus any enums declared outside classes, however:
* Implementation files should contain the Doxygen documentation for class methods.
  This keeps the documentation next to the implementation (making it easier to keep up-to-date).
  It also makes the headers easier to read.
* If a class provides high-level functionality, consider writing some user-guide-style documentation
  in the User's Guide (in :file:`doc/userguide.rst`) or a tutorial (in :file:`doc/tutorials/`).
  Tutorials should include a working example that is run as a CTest test.
  The code in the example should be referenced indirectly by the tutorial so that
  the the exact code that is tested appears as the text of the tutorial.
* In reStructuredText documents, you should use the doxylinks_ module to link to
  the Doxygen documentation *when appropriate*.
  Examples:
  ``:smtk:`UUID``` produces this link: :smtk:`UUID` while the
  ``:smtk:`Manager <smtk::attribute::Manager>``` variant can produce
  links (:smtk:`Manager <smtk::attribute::Manager>` in this case) whose text varies from the classname
  or whose classnames are ambiguous because of namespaces.
  The leading ``:smtk:`` names the tag file holding the class and function definitions;
  other third-party-library tag files may be added in the future.

  You will be tempted to make every word that is a classname into a Doxygen link; do not do this.
  Instead, provide a Doxygen link at the first occurrence of the classname in a topic's
  discussion — or at most in a few key places. Otherwise the documentation becomes difficult to read
  due to conflicting text styles.
* In reStructuredText, when you wish to show code in-line but it is inappropriate to link to Doxygen documentation,
  use the ``:cxx:`` role for C++ (e.g., :cxx:`if (foo)`), the ``:file:`` role for paths to files (e.g., :file:`doc/index.rst`), and so on.
  See the `documentation for roles in reStructuredText`_ for more information.
* Note that the user's guide and tutorials are both included in the top-level :file:`doc/index.rst` file
  parsed by Sphinx.
  Several extensions to Sphinx are used and these are configured in :file:`doc/conf.py`.

To get started documenting your code, you should at least have doxygen_ and graphviz_ installed.
These are available using Homebrew_ on Mac OS X, your Linux distribution's package manager, or by binary
installer from the source maintainer on Windows.

Additionally there are a number of Python packages that provide Sphinx, docutils, and other packages required
to generate the user's guide.
These packages can all be installed with pip:

.. highlight:: sh
.. code-block:: sh

  # The basic utilities for processing the user's guide:
  sudo pip install docutils
  sudo pip install Sphinx
  # For linking to external Doxygen docs:
  sudo pip install sphinxcontrib-doxylinks
  # For creating inline class docs from Doxygen XML:
  sudo pip install breathe
  # For the default theme:
  sudo pip install sphinx-rtd-theme
  # For syntax highlighting:
  sudo pip install Pygments
  # For activity diagrams:
  sudo pip install sphinxcontrib-actdiag

If you are unfamiliar with the documentation packages here, see these links for examples of their use
(or use SMTK by example):

* `Sphinx Table of Contents <http://sphinx-doc.org/contents.html>`_
* `Sphinx conf.py configuration <http://sphinx-doc.org/config.html>`_
* `reStructuredText primer <http://sphinx-doc.org/rest.html>`_
* `Doxygen commands <http://www.stack.nl/~dimitri/doxygen/manual/index.html>`_


.. _doxygen: http://doxygen.org/
.. _doxylinks: https://pypi.python.org/pypi/sphinxcontrib-doxylink
.. _graphviz: http://graphviz.org/
.. _Homebrew: http://brew.sh/
.. _Sphinx: http://sphinx-doc.org/
.. _reStructuredText: http://docutils.sourceforge.net/rst.html
.. _VTK: http://vtk.org/
.. _documentation for roles in reStructuredText: http://sphinx-doc.org/markup/inline.html

To-do list
==========

Finally, if you are looking for a way to contribute,
helping with the documentation would be great.
A list of incomplete documentation (or incomplete features)
is below.
You can also look on the SMTK issue tracker for things to do.

.. todolist::
