String tokens are now constexpr
-------------------------------

The :smtk:`smtk::string::Token` class, which stores the hash
of a string rather than its value, now includes a ``constexpr``
hash function (the algorithm used is a popular one named "fnv1a")
so that strings can be tokenized at compile time. Previously,
the platform-specific ``std::hash_function<std::string>`` was
used.

This change makes it possible for you to use switch statements
for string tokens, like so:

.. code-block:: c++

   using namespace smtk::string::literals; // for ""_hash
   smtk::string::Token car;
   int hp; // horsepower
   switch (car.id())
   {
     case "camaro"_hash: hp = 90; break;
     case "mustang"_hash: hp = 86; break;
     case "super beetle"_hash: hp = 48; break;
     default: hp = -1; break;
   }

The hash algorithm will generate hashes of type ``std::size_t``
but only supports 32- and 64-bit platforms at the moment.
Note that because the string manager uses a serialization helper
to translate serialized hash values (this was previously required
since ``std::hash_function<>`` implementations varied), reading
tokens serialized by a 32-bit platform on a 64-bit platform will
not present problems. However, reading 64-bit hashes on a 32-bit
platform is not currently supported; it may be in a future release
but we do not foresee a need for it.

Note that if a string is tokenized at compile time (i.e., by
using ``"string"_hash`` instead of ``smtk::string::Token``'s
constructor), its string value will not be added to the
:smtk:`smtk::string::Manager` instance and can thus not be
retrieved at run time unless some other piece of code adds it.
Instead a ``std::invalid_argument`` exception will be thrown.
