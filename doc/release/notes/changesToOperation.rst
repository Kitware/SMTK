Changes to Operation
====================

Deprecated Key struct
---------------------

The ``Key`` struct has been deprecated in favor of ``BaseKey`` which contains options to alter
certain behaviors of ``Operation::operate()`` when running an operation from within another
operation's ``operateInternal()`` synchronously.

``Key`` derives from ``BaseKey`` so that the legacy API can still be satisfied, but any dependent
code should be refactored to use ``Operation::childKey()`` instead

See the user documentation for more details about the options that can be passed to
``Operation::childKey()`` in the "Resource Locking" section.
