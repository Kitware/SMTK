Observers and debugging
-----------------------

Now :smtk:`smtk::common::Observers` takes a second template parameter:
a boolean indicating whether to print debug messages or not.
This does away with the previous macro (which printed messages for *all*
observers – resource, operation, etc.) in favor of messages that can
be enabled for a single specialization. See the :ref:`smtk-tips` section
of the user's manual for more information.
