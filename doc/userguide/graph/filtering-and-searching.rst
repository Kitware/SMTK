Filtering and Searching
=======================

It is possible to ask the resource to identify some nodes using a
string that specifies search criteria (i.e., a filter). The
default implementation provided by
`smtk::graph::Resource::queryOperation` allows users to filter
nodes based on their component type-name as well as the existence and
values of properties attached to them.  For example, a geometric
model of a motor will have many model faces that might each be
marked with properties to indicate which are bearing surfaces,
which are fastener or alignment surfaces, which surfaces will be
in contact with coolant or fuel, etc.

In order to allow user interface components to only show relevant
model entities, the resource's :smtk:`queryOperation
<smtk::resource::Resource::queryOperation>` method accepts strings in
the following format:

    ``node-typename`` ``[`` property-type [ ``{`` property-name [
    ``=`` property-value ] ``}`` ]

where

+ ``node-typename`` specifies matches to names returned from any
  node's ``typeName()`` method. The type name may be

    + an exact match for a component's type-name (with no enclosing quotes);
    + a single-quoted type-name;
    + a forward-slash-enclosed regular expression matching one or more type names; or
    + ``*`` or ``any`` to indicate any graph node should be considered.

+ ``property-type`` is one of the following string literals
  ``string``, ``floating-point``, ``integer``.
+ ``property-name`` is either a single-quoted name or a slash-quoted
  regular expression (i.e., a regular expression surrounded by forward
  slashes such as ``/(foo|bar)/)``.
+ ``property-value`` is one of the following
    + a single, single-quoted string value to match (when searching
      for string properties),
    + a single, slash-quoted regular expression to match (when
      searching for string properties by regular expression),
    + a single, unquoted integer or floating point value to match
      (when searching for properties of those types), or
    + a tuple (indicated with parentheses) of values, as specified
      above, to match. Note that this implies the property must be
      vector-valued and the length must match the specified tuple's
      length in order for a match to be successful.

Whitespace is allowed anywhere but is treated as significant if it is
inside any quoted string value or regular expression.

Note that single quotes are used because these filter strings will
appear in XML and/or JSON serializations that use double-quotes to
mark the start and end of the query string.  The examples below
include the double-quotes around the query as a reminder.

For regular expressions, the c++11 standard library is used to search
for matches; the syntax must be accepted by the std::regex constructor
and std::regex_search() must return true when passed property names or
values in order for the corresponding entity to be included in
filtered results.

See the documentation for `smtk::resource::Resource` Filtering and
Searching for examples of filtering on property types and values.
