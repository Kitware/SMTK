Operation System
================

Exception handling
------------------

Since April 2025, exceptions thrown from within :smtk:`smtk::operation::Operation::operateInternal`
have been caught to avoid situations where resource locks are never freed because the stack frame
holding the lock was unwound. We now catch exceptions thrown by operation observers and handlers
for the same reason.
