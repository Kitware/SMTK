Adding data to SMTK
===================

There are two tasks related to adding data to SMTK:

1. Storing data in the SMTK repository; and
2. Embedding data in SMTK libraries, plugins, and/or python modules.

Data in the repository
~~~~~~~~~~~~~~~~~~~~~~

SMTK uses Git Large File Storage (LFS) to hold large files of any
format, whether it be binary or ASCII.
Using Git LFS is a way to reduce the amount of data developers
must download to just that in active use by the branch of SMTK
they are working on.

If you need to add a large file to SMTK,

+ Ask SMTK developers the best approach to hosting the data
  early in the process to avoid extra work.
+ If Git LFS is the proper way to add data, ensure that you
  have run ``git lfs install`` in your local SMTK clone and
  use ``git lfs status`` to verify that large files you are
  about to commit will be stored with LFS.

Embedding data in SMTK targets
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SMTK provides a CMake macro, ``smtk_encode_file()``, that will
transcribe a repository file into a C++ literal, C++ function,
or Python block-quote. This generated file can be built into
targets such as libraries, plugins, or Python modules – allowing
you to load content without needing to locate where on a user's
filesystem it might reside.

.. include:: ../../../CMake/SMTKOperationXML.cmake
   :start-line: 1
   :end-before: #]====
