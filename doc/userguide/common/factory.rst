Factory
=======

SMTK's factory pattern (:smtk:`Factory <smtk::common::Factory>`)
allows consuming code to register types that share a common base class
with a factory instance at runtime and subsequently construct
instances of these types using the type (`Type`), type name
(`smtk::common::typeName<Type>()`), or type index
(`typeid(Type).hash_code()`). Upon declaration, a factory takes as
arguments the base class type and a list of input parameter types
passed to the generated classes upon construction. As a convenience,
each of the above construction modes has an associated Interface to
provide a uniform API when constructing objects.

Factories return unique pointers to objects they construct.
Because shared pointers can be constructed from unique pointers,
the factory pattern can be used in both cases where single and
shared ownership are needed.

An example that demonstrates the prinicples and API of this pattern
can be found at `smtk/common/testing/cxx/UnitTestFactory.cxx`.

Instances
=========

As an extension to the Factory pattern, SMTK also provides
a templated :smtk:`Instances <smtk::common::Instances>` class.
The Instances class not only tracks types that can be constructed
but also manages instances that have been constructed by holding
a shared pointer to each object it creates.

The Instances class can be instructed to release any shared pointer
it owns which — assuming no other shared pointers reference the same
object — will delete the object.

Upon construction and release of objects its manages, the Instances
object invokes Observers (covered later in this section).
Observing the construction and imminent deletion of objects allows
user interfaces an opportunity to present summaries of the system
state.

Instances takes the same set of template parameters as the Factory:
the common base type and signatures used to construct instances.
