CMake Policies
==============

Because of CMake policy CMP0115 (source file extensions must be explicit),
when passing test names to the ``smtk_add_test`` macro, be sure to include
the filename extension (such as ``.cxx``).
