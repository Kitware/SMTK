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

A token can always provide its source string data, but equality
and inequality comparisons are done by comparing an integer token identifier.

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

The token IDs are stored in a class-static dictionary called
the string :smtk:`Manager <smtk::string::Manager>`.
This dictionary is what allows Tokens to return the original
string data while only holding a token ID.

The dictionary can be serialized-to and deserialized-from JSON.
Individual token instances are serialized simply as their integer identifier.
