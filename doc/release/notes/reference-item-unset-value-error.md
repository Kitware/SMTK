#### Unset Value Error for Reference Item Iterator
A custom exception is now thrown when an attempt is made to
dereference an iterator to an unset reference item. This exception can
be caught by consuming code (for an example, see the
unitUnsetValueError test). An isSet() method is now available on
ReferenceItem's const_iterator to check the iterator's validity prior
to dereferencing.
