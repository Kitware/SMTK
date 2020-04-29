## Changes to Common
### Updates to typeName() logic
The `smtk::common::typeName<>()` function's logic has been modified to no longer
check for the existence of a `virtual std::string typeName() const` method
associated with objects passed as the template parameter. The check was removed
to avoid the case where derived classes were inheriting their parent's typename,
resulting in unexpected behavior (instead of either a compile-time error or a
run-time exception).

### Updates to Observers
`smtk::common::Observers` functors are now automatically removed from the
`smtk::common::Observers` instance when their key goes out of scope. The original
functionality requiring that the functor be manually removed can be recovered by
calling `release()` on the functor's Key.
