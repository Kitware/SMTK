Type container now uses string tokens as keys
---------------------------------------------

Previously, :smtk:`smtk::common::TypeContainer` indexed an
object of type ``Type`` by ``typeid(Type`).hash_code()``.
However, on macos this caused issues where the hash code
varied depending on the library calling methods on the
type container. To fix this, the type container now
uses :smtk:`smtk::string::Hash` ids (as computed by
constructing a string Token) of the typename. This is an
internal change and does not affect the public API of the
template class; no developer action should be required.
