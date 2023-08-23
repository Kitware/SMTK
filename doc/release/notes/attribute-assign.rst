Attribute System
----------------

Item assignment return type is changing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Previously, when you called ``smtk::attribute::Item::assign()``, it would return
a boolean value indicating success or failure.
However, in the case of a successful assignment, there was no way to determine if
the target item and/or its children were actually modified.
Now, this method returns an object, :smtk:`smtk::common::Status`,
that may be queried for both ``success()`` and ``modified()``.

This change is needed so that task adaptors (and others) can determine whether
to mark :smtk:`smtk::task::SubmitOperation` tasks as needing to be re-run.
