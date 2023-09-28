Type Container
==============

SMTK's type container (:smtk:`TypeContainer
<smtk::common::TypeContainer>`) is a generic container for storing
single instances of types. The contained types do not have any
requirements for sharing a common base class; as a result, the
container does not require any parameters upon construction. Because
of this, there is no run-time method for discovering the contents of a
container, only for probing whether a given type is contained. As a
tradeoff, element access from the container is templated on the type
to be returned (the element type is used as a key to retrieve the
element and subsequently perform the appropriate conversion). If an
element is default-constructible, an attempt to access an element that
is not currently in the container will result in the construction of
an instance of that object.

An example that demonstrates the prinicples and API of this pattern
can be found at `smtk/comon/testing/cxx/UnitTestTypeContainer.cxx`.

Run-time Type Container
=======================

:smtk:`smtk::common::RuntimeTypeContainer` is a subclass of TypeContainer.
The base TypeContainer class can only hold a single object of a given type.
When applications handle many objects that share a common base type (i.e., whose
complete type is unknown since only a pointer to a base type is held),
there was no way to insert these distinct objects into a TypeContainer even if
their complete types were distinct.

To resolve this issue, the RuntimeTypeContainer class allows you to insert
objects by their base type but use a "declared type-name" as the key.
As long as these declared type-names are unique, multiple objects sharing the
base type can be held by the container simultaneously.
