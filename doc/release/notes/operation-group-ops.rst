Operation System
----------------

A new ``smtk/operation/GroupOps.h`` header has been added that provides
methods to apply an :smtk:`operation group <smtk::operation::Group>` to
a container of :smtk::`smtk::resource::PersistentObject` instances.
The function will identify the proper set of operations which associate
to each persistent object in the container and, if all are able to operate,
will launch these operations with their associated objects.

In the event some objects cannot associate to any operation in the group
or some operations are unable to operate as created, a lambda provided to
the function is invoked to obtain feedback from users before launching or
aborting.

A specific function is provided to invoke the above for the :smtk:`smtk::operation::DeleterGroup`
along with a Qt-based function, :smtk:`smtk::extension::qtDeleterDisposition`
that can be passed as the final argument for querying users about deleting
objects with dependencies.

This refactors and improves code from :smtk:`smtk::extension::qtResourceBrowser`,
which would only launch a single operation from the deleter-group (requiring it to
associate to every persistent object provided).
