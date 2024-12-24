Operation system
================

Some operation members now less private
---------------------------------------

Some members of :smtk:`smtk::operation::Operation` are now public or
protected (rather than protected or private) in order to allow Python
operations the ability to invoke a nested operation.
