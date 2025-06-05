Operation System
================

Handlers for specific operation instances
-----------------------------------------

You may now add and remove handlers to an instance of an operation.
A handler is a function to be invoked upon the next completion of
one instance of an operation object (regardless of whether the result
indicates success or failure).

Handlers are function objects with a signature similar to observers
(see :ref:`operation-observers`)
except that they are only called upon completion of an operation:
handlers may not cancel the operation and are not passed an ``EventType``
(only the operation and its result).
Handlers, like observers, are invoked at a time when all the resource
locks required for the operation are held.

Handlers are invoked on the thread in which the operation is run (unlike
observers, which are invoked on the main/GUI thread in Qt applications).

Handlers are invoked only for the instance of the operation they are
added to; if you create multiple operations of the same type and add
a handler to one, only that instance will have the handler called.

Handlers are invoked zero or one times at most.
Operation handlers are removed each time the operation is invoked;
you are responsible for adding handlers for each invocation.
It is acceptable for a handler to add itself to the operation which
invoked it (as the container of handlers is copied and cleared before
any handlers are invoked).
