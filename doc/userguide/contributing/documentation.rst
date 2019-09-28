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
  ``:smtk:`Resource <smtk::attribute::Resource>``` variant can produce
  links (:smtk:`Resource <smtk::attribute::Resource>` in this case) whose text varies from the classname
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
  sudo pip install sphinxcontrib-doxylink
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
.. _documentation for roles in reStructuredText: http://sphinx-doc.org/markup/inline.html
