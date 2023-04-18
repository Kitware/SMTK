Default observer priority
-------------------------

The :smtk:`smtk::common::Observers` template now uses a default priority
of 0 instead of ``std::numeric_limits<int>::lowest()`` (which is a negative number).
Any code that inserts observers using the signature that does not take an explicit
priority may now invoke that observer earlier than in previous releases of SMTK.
If you wish to maintain the old behavior, you must now explicitly pass a priority.

The :smtk:`smtk::common::Observers` template now provides methods
named ``defaultPriority()`` and ``lowestPriority()`` for your convenience.

This change was made to facilitate observers provided by SMTK that need to ensure
they are the last invoked after an operation since they release objects from managers
and may invalidate the operation result object.
