## Add a group for declaring operations internal

By default, any operation that is registered with an operation manager
will be presented in the list of available operations retrieved by the
AvailableOperations class. An operation group,
smtk::operation::InternalGroup, has been added so developers can
explicitly mark their operation as an internal, or private,
operation. Operations added to the InternalGroup will not be presented
to the user as available.
