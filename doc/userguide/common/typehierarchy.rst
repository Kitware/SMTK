Type Hierarchy
==============

SMTK provides a :smtk:`smtk::common::typeHierarchy`) template
that will populate a container with type-names (computed
via :smtk:`smtk::common::typeName`) of the class and all its
inherited members.
This template makes use of the ``smtkSuperclassMacro``, so only
those classes which have the ``Superclass`` type-alias present
will be detected.

This free function is used by new methods on
:smtk:`smtk::resource::PersistentObject` (and any other class
hierarchy that uses ``smtkTypenameMacro()`` and ``smtkTypnameMacroBase()``)
to determine whether a string-token matches any of the object's inherited
types and, if so, how many "generations" of inheritance the object lies
from the given base class name.

An example that demonstrates the prinicples and API of this pattern
can be found at `smtk/comon/testing/cxx/UnitTestTypeHierarchy.cxx`.
