.. _smtk-common-paths:

Locating auxiliary files
=======================

Applications and plugins often install workflows, attribute templates, Python
scripts, and configuration files alongside their libraries. These are
*auxiliary files*: files used to configure or support the application, distinct
from the SMTK Resources managed by ``smtk::resource::Manager``.

Use :smtk:`smtk::common::Paths` to locate these files at runtime instead of
embedding an installation prefix or assuming that the executable is in a
particular directory. ``auxiliarySearchPaths()`` returns the candidate
directories, and ``findAuxiliaryPath(relativePath)`` finds an existing file or
directory beneath them.

Installation layouts and search order
-------------------------------------

Candidate directories are returned in this order:

1. ``<app>.app/Contents/Resources/smtk``, when the running application has a
   macOS app bundle.
2. ``<library-directory>/../share/smtk``, relative to the loaded SMTK library.
3. ``<top-level-directory>/share/smtk``, relative to SMTK's detected top-level
   directory.

For a conventional Linux installation with libraries in ``<prefix>/lib`` or
``<prefix>/lib64``, the library-relative candidate is ``<prefix>/share/smtk``.
For a Windows installation with DLLs in ``<prefix>/bin``, it is also
``<prefix>/share/smtk``. The bundle candidate is omitted on platforms without
app bundles. When multiple candidates identify the same normalized path, only
the first occurrence is returned.

For example, install a mesh template at
``<prefix>/share/smtk/workflows/example/mesh.smtk``. In a packaged macOS
application, place the same file at
``<app>.app/Contents/Resources/smtk/workflows/example/mesh.smtk``. In both cases,
the lookup argument is ``workflows/example/mesh.smtk``.

The library-relative candidate assumes the library directory is immediately
beneath the install prefix. A layout such as ``lib/x86_64-linux-gnu`` needs
another candidate, such as the detected top-level directory, to reach the
installation's ``share/smtk`` directory.

Lookup behavior
---------------

Search paths are normalized without requiring their directories to exist.
``findAuxiliaryPath()`` checks each candidate for the particular file or
directory requested; it does not select one root for all future lookups.
Thus, if an app bundle contains some auxiliary files but lacks a particular
template, lookup can fall back to that template in the install tree.

Supply a path relative to an auxiliary directory. Empty or rooted paths and
paths whose normalized form escapes the directory through ``..`` are rejected.
When no match is found, C++ returns an empty ``std::filesystem::path`` and
Python returns ``None``. Check for this result before opening the file or
passing its path to an operation.

C++ example
-----------

.. code-block:: c++

   #include "smtk/common/Paths.h"

   #include <filesystem>
   #include <stdexcept>

   std::filesystem::path meshTemplate()
   {
     smtk::common::Paths paths;
     auto filename = paths.findAuxiliaryPath("workflows/example/mesh.smtk");
     if (filename.empty())
     {
       throw std::runtime_error("The example mesh template is not installed.");
     }
     return filename;
   }

Python example
--------------

Both methods are exposed on ``smtk.common.Paths``. Search directories and
successful lookup results are returned as ``pathlib.Path`` objects. The lookup
accepts either a string or a ``pathlib.Path`` argument.

.. code-block:: python

   from pathlib import Path
   import smtk.common

   paths = smtk.common.Paths()
   relative = Path("workflows/example/mesh.smtk")
   filename = paths.findAuxiliaryPath(relative)
   if filename is None:
       for directory in paths.auxiliarySearchPaths():
           print("Searched:", directory / relative)
       raise FileNotFoundError("The example mesh template is not installed.")

   print("Using template:", filename)

When diagnosing missing files, compare the returned candidate directories with
where the application or plugin installs its auxiliary files. This also helps
detect an application loading SMTK from a different installation than expected.
