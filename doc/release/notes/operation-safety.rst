Operation-system Changes
------------------------

We continue to identify and eliminate inconsistent behavior in asynchronous operations.
At some point in the future, :smtk:`smtk::operation::Operation::operate()` will either
be removed or see its signature modified to no longer return an operation Result; the
expected pattern will be for all users to launch operations at that point and use an
observer or handler functor to examine the result (while resource locks are held by
the operation).

In the meantime, we have introduced a new way to invoke an operation: rather than
calling ``operate()``, which returns a result, you should either launch an operation
or invoke ``safeOperate()`` which accepts a functor that will be evaluated on the
result. The method itself returns only the operation's outcome. This is done to prevent
crashes or undefined behavior (that can occur if you inspect the result without holding
locks on all the involved resources). The handler (if you pass one) is invoked before
the operation releases resource locks. Note that ``safeOperate()`` blocks until the
operation is complete and should not be invoked on the main thread of a GUI application
since it can cause deadlocks.
