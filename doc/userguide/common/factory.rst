Factory
=======

SMTK's factory pattern (:smtk:`Factory <smtk::common::Factory>`)
allows consuming code to register types that share a common base class
with the factory insetance at runtime and subsequently construct
instances of these types using the type (`Type`), type name
(`smtk::common::typeName<Type>()`), or type index
(`typeid(Type).hash_code()`). Upon declaration, a factory takes as
arguments the base class type and a list of input parameter types
passed to the generated classes upon construction. As a convenience,
each of the above construction modes has an associated Interface to
provide a uniform API when constructing objects.

An example that demonstrates the prinicples and API of this pattern
can be found at `smtk/comon/testing/cxx/UnitTestFactory.cxx`.
