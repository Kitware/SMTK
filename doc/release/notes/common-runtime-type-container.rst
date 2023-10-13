Changes to common infrastructure
--------------------------------

TypeContainer changes
~~~~~~~~~~~~~~~~~~~~~

A few minor changes were made to :smtk:`smtk::common::TypeContainer`:
+ Methods and variables that were previously private are now protected so that
  this class can be subclassed.
+ The wrapper class used to store objects in the type container now provides a
  string token holding the type-name of the inserted type.
  This is used by the new :smtk:`smtk::common::RuntimeTypeContainer` subclass described below.
+ The ``insert_or_assign()`` method has been renamed ``insertOrAssign()``
  to be consistent with the rest of SMTK.
  The original name is deprecated and will be removed in a future version of SMTK.

New RuntimeTypeContainer
~~~~~~~~~~~~~~~~~~~~~~~~

:smtk:`smtk::common::RuntimeTypeContainer` is a new subclass of TypeContainer.
The base TypeContainer class can only hold a single object of a given type.
When applications handle many objects that share a common base type (i.e., whose
complete type is unknown since only a pointer to a base type is held),
there was no way to insert these distinct objects into a TypeContainer even if
their complete types were distinct.

To resolve this issue, the RuntimeTypeContainer class allows you to insert
objects by their base type but use a "declared type-name" as the key.
As long as these declared type-names are unique, multiple objects sharing the
base type can be held by the container simultaneously.

See the class and its test for detailed documentation.
