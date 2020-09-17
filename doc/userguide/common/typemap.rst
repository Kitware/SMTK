Type Map
========

SMTK's type map (:smtk:`TypeMap <smtk::common::TypeMap>`) is a
generalized map for storing and accessing data using both a type and a
user-defined key type. Unlike :smtk:`TypeContainer
<smtk::common::TypeContainer>`, a type map has an additional key
index; a TypeMap can therefore contain multiple instances of a
single type, each accessed by a unique key. This additional level of
misdirection gives the type map more flexibility than a type
container, at the expsense of an additional lookup phase. In addition,
serialization routines are provided for the type map that facilitate
its transcription to and from json.

An example that demonstrates the prinicples and API of this pattern
can be found at `smtk/comon/testing/cxx/UnitTestTypeMap.cxx`.
