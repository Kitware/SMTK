CMake string encoding
---------------------

The ``encodeStringAsCVariable`` CMake function in ``CMake/EncodeStringFunctions.cmake``
(which is used by ``smtk_encode_file()``) has been modified to generate a C++11 raw
string literal rather than an escaped C98 string constant. This change was made for
performance reasons (CMake would deplete its heap space trying to process files more
than a few megabytes).

If you use the functions above, be aware that you can no longer use the generated
headers with a C compiler or a C++ compiler which does not support C++11 raw string
literals.
