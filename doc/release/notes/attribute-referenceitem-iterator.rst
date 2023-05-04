Attribute system
----------------

Accessing ReferenceItem Iterator Values
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``ReferenceItem::const_iterator`` class now provides
a templated ``as()`` method that will dynamically cast
shared pointers to a child class. Note that this method
will return null shared pointers when the object's type
is mismatched.
