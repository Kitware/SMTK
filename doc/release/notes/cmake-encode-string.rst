CMake string encoding
---------------------

The way SMTK encodes the contents of files into its libraries has changed
in several regards.

Newly deprecated functions
~~~~~~~~~~~~~~~~~~~~~~~~~~

The CMake functions in ``CMake/EncodeStringFunctions.cmake`` (used by ``smtk_encode_file()``)
have been replaced by a C++ executable ``utilities/encode/smtk_encode_file.cxx`` to allow
processing of large files efficiently. These functions are now deprecated.

If you use the functions in ``CMake/EncodeStringFunctions.cmake``, be aware that these
are deprecated and will be removed in a future version. If you have small files and
wish to use C++11 literals, be aware that you can no longer use the generated
headers with a C or C++ compiler unless it supports C++11 raw string literals.

Refactored function
~~~~~~~~~~~~~~~~~~~

The ``smtk_encode_file()`` CMake macro now calls a C++ binary of the same name (whose source
is located in ``utilities/encode/``). The new utility can generate files containing

+ a python block quote (as before);
+ a C++11 raw string literal (rather than the previous, escaped C98 string constant); or
+ a C++11 function that returns a ``std::string`` (all new).

The latter change (C++11 function output) was made to accommodate encoding files
larger than 64kiB on windows, which prohibits arbitrary-length literals. The CMake
implementation of string splitting was rejected for performance reasons (CMake would
deplete its heap space trying to process files more than a few megabytes).

The new macro and executable also introduce a change in behavior;
previously, if you invoked ``smtk_encode_file("foo/bar.xml" …)`` inside the
subdirectory ``baz/xyzzy`` of your project's source, the generated file would
be at ``baz/xyzzy/bar_xml.h``. Now the path to the generated file will be
``baz/xyzzy/foo/bar_xml.h``.
This pattern is used by SMTK for operations and icons; if your external
library provides these, you will need to adjust your include directives.

Removal of previously deprecated functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Finally, the deprecated CMake functions ``smtk_operation_xml`` and ``smtk_pyoperation_xml``
from ``CMake/SMTKOperationXML.cmake`` have been removed.
