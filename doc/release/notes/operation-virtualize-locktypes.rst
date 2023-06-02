Operation System
----------------

Operations can customize what resources they lock
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The base :smtk:`Operation <smtk::operation::Operation>` class now provides
a virtual method, ``identifyLocksRequired()`` to allow subclasses to customize
the default set of resources to be locked and their lock levels (read vs. write).
This allows operations that may need to lock components related (say by
:smtk:`Links <smtk::resource::Links>`) to external resources to include those
resources in the operation's lock set.
