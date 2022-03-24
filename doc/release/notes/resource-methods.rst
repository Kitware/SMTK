New methods on smtk::resource::Resource
---------------------------------------

You can now ask a resource for raw pointers to components
via the ``components()`` and the (templated) ``componentsAs<X>()``
methods.
Use these methods **only** when
(1) you have a read or write lock on the resource (i.e., you are
    inside an operation) so that no other thread can remove the
    component; and
(2) you are sure that the component will not be removed from the
    resource for the duration of your use of the pointer (in the
    case where a write lock is held and components may be removed).

Note for developers
^^^^^^^^^^^^^^^^^^^

If you subclass :smtk:`smtk::resource::Resource`, you should
consider overriding the ``components()`` method to provide an
efficient implementation as it can improve performance as the
number of components grows large.
You may also wish to consider changing any subclasses of
:smtk:`smtk::resource::Component` to refer back to their parent
resource using a :smtk:`smtk::common::WeakReferenceWrapper`
and providing a method to fetch the raw pointer to the parent
resource from the weak-reference-wrapper's cache.
This will improve performance when many components must be asked
for their parent resource as the overhead of locking a weak pointer
and copying a shared pointer can be significant.
However, note that any method returning a raw pointer will not
be thread-safe. This method is intended solely for cases where
a read- or write-lock is held on the resource and the algorithm
can guarantee a shared pointer is held elsewhere for the duration.

If you maintain an operation that needs to be performant with
large numbers of components, consider using these methods to
avoid the overhead of shared-pointer construction.
