Attribute Resource Changes
==========================

Values methods for Items derived from ValueItems
------------------------------------------------

The C++ ValueItemTemplate class provided methods of setting multiple values of the item using iterators.  Unfortunately these methods could not be made available in the Python wrapped API.  Instead we have added *setValues* and *values* methods to the pybind11 wrapping for DoubleItem, IntItem, and DoubleItem which allows the use of Python arrays to set (and get) all of the values of the Item.
