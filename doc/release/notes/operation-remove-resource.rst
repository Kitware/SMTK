Operation System
----------------

Removing Resources
~~~~~~~~~~~~~~~~~~

The :smtk:`smtk::operation::RemoveResource` operation now accepts multiple
resources to remove as a convenient alternative to repeatedly running the
operation with a different resource each time. (The operation was programmed
to do this but a bug in its allowed associations prevented it from being
passed multiple inputs.)
