## Changes to Common
### Updates to typeName() logic
The `smtk::common::typeName<>()` function's logic has been modified to no longer
check for the existence of a `virtual std::string typeName() const` method
associated with objects passed as the template parameter. The check was removed
to avoid the case where derived classes were inheriting their parent's typename,
resulting in unexpected behavior (instead of either a compile-time error or a
run-time exception).
