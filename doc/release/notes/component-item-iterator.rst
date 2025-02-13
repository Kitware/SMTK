Attribute System
================

Reference item iterator support
-------------------------------

Previously, the component item implementation only provided partial
iterator support, rendering them unusable with ranged-for loop and
the various algorithms included in the C++ standard library. The
`ComponentItem` class now implements the necessary API for returning a
`const_iterator` as expected by most utility templates dealing with
containers.
