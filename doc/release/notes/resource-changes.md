## Changes to SMTK Resource

* queryOperation now returns a functor that takes in a const smtk::resource::Component& instead of a const smtk::resource::ConstComponentPtr&

* Resources now have Query functors. Queries are functors designed to incrementally augment the API of a Resource at runtime via registration.

* Derived Resources can now be defined entirely in Python.
