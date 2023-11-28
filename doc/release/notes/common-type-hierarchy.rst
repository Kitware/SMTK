Changes to common infrastructure
--------------------------------

Type hierarchy reflection
~~~~~~~~~~~~~~~~~~~~~~~~~

SMTK has long provided ``smtkTypeMacro()`` and ``smtkSuperclassMacro()``
in order to provide classes with type-aliases and virtual methods that
allow reflection of an object's type.
This has now been extended to provide (when a ``Superclass`` type-alias
is present) the entire inheritance hierarchy.

In addition to the virtual ``typeName()`` method, each class that uses
``smtkTypeMacro()`` or ``smtkTypeMacroBase()`` now provides

* ``matchesType(smtk::string::Token baseType)`` – which returns true
  if the object is or inherits the given base type
* ``classHierarchy()`` – which returns a vector of string-tokens
  holding the type-names of the object and its base classes.
* ``generationsFromBase(smtk::string::Token baseType)`` – which returns
  an integer indicating the number of "hops" along the inheritance tree
  required to get from the object's type to the given base type.

See ``smtk/common/testing/cxx/UnitTestTypeHierarchy.cxx`` for examples.
