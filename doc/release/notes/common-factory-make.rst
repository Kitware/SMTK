Factory API for shared pointers
-------------------------------

The :smtk:`smtk::common::Factory` template now provides method
names that begin with ``make`` and which return shared pointers
(in addition to the ``create`` methods which return unique pointers).

This change was made to accommodate some compilers (e.g., gcc 11)
that are unable to cast unique pointers with an explicit deleter into
shared pointers.
