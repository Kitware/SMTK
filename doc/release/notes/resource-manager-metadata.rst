Improved ``smtk::resource::Metadata``
-------------------------------------

The :smtk:`smtk::resource::Metadata` class constructor now requires
create, read, and write functors which take an
:smtk:`smtk::common::Managers` instance as input so that creating,
reading, and writing resources can make use of any available
application-provided manager objects.

If you had any resource subclasses that provided these functors,
you must update to the new signature.
This is a breaking change.

Be aware that the operation and operation manager classes now accept
an :smtk:`smtk::common::Managers` instance.
If provided to the operation manager, all operations it creates will
have the managers object set (for use by operations).
This is the preferred way for applications to pass information to operations.
Using this method allows operations to be used in several applications
with minimal dependencies on application-specific methods and structures.
