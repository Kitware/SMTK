Motivation
==========

String equality tests can be slow, but using integer values
to enumerate names for things (e.g., labels, tags, enum and
event names, keywords in an input deck, etc.) can prevent
plugins from extending the set of names with new values and
can also lead to errors when values are written to files
that must be maintained even as the set of valid values is
modified.

To ameliorate the situation, SMTK provides some utility
classes for string tokenization. Treating some set of fixed
strings as tokens allows compact storage (the token ID can
be stored in place of a potentially long string) and easy
extension (since files can contain the mapping of token IDs
to strings and those mappings can be reconciled at runtime).

Concepts
========

SMTK provides a string :smtk:`Token <smtk::string::Token>` class
to represent a string with an integer ID;
a token can be constructed from a ``std::string`` or
from a string literal like so:

.. code:: c++

   #include "smtk/string/Token.h"
   using smtk::string::Token;

   Token a = "foo";
   Token b = """bar"""_token;
   Token c = std::string("baz");

A token can provide the source string data if it was constructed
by passing a string to be hashed, but may throw an ``std::invalid_argument``
exception if then token was constructed by passing the hash code
directly (i.e., with ``smtk::string::fromHash()``). So, in the example
above, ``a.data()`` and ``c.data()`` will return "foo" and "baz",
respectively, but ``b.data()`` will throw an exception unless "bar" was
added to the string-token manager elsewhere.

Equality comparisons are done by comparing integer token identifiers (hash
codes) and are thus fast. Inequality comparisons resort to string-value
comparisons and thus may be slow for large strings with identical prefixes.

.. code:: c++

  std::cout << "a is \"" << a.data() << "\" with id " << a.id() << "\n";
  // prints: a is "foo" with id 9631199822919835226
  std::cout << "b is \"" << b.data() << "\" with id " << b.id() << "\n";
  // prints: b is "bar" with id 11474628671133349555

  a == b; // This is fast since it only compares token IDs.
  a != b; // This is fast since it only compares token IDs.
  a < b; // This is slow since it compares underlying strings.

As noted in the example above, less-than and greater-than operators are
slow because they compare the underlying strings.
This preserves lexographic ordering when you store tokens in
ordered containers.

Switch statements
=================

As of SMTK 22.10, string tokens may be constructed via a
``constexpr`` literal operator named ``_hash``. This makes it possible
for you to use switch statements for string tokens, like so:

.. code-block:: c++

   using namespace smtk::string::literals; // for ""_token
   smtk::string::Token car;
   int hp; // horsepower
   switch (car.id())
   {
     case "camaro"_hash: hp = 90; break;
     case "mustang"_hash: hp = 86; break;
     case "super beetle"_hash: hp = 48; break;
     default: hp = -1; break;
   }

String source data
==================

Token IDs and their corresponding string source-data are stored in a
class-static dictionary called the string
:smtk:`Manager <smtk::string::Manager>` as mentioned above.
This dictionary is what allows Tokens to return the original
string data while only holding a token ID.

The dictionary can be serialized-to and deserialized-from JSON.
Individual token instances are serialized simply as their integer identifier.
Because platforms may tokenize strings differently, the dictionary
provides a "fallback map" constructed during deserialization to
translate hash codes from other platforms.

Token hashing algorithm
=======================

The hash algorithm will generate hashes of type ``std::size_t``
but only supports 32- and 64-bit platforms at the moment.
Note that because the string manager uses a serialization helper
to translate serialized hash values (this was previously required
since ``std::hash_function<>`` implementations varied), reading
tokens serialized by a 32-bit platform on a 64-bit platform will
not present problems. However, reading 64-bit hashes on a 32-bit
platform is not currently supported; it may be in a future release
but we do not foresee a need for it.
