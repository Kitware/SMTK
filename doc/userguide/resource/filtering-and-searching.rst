Filtering and Searching
=======================

It is possible
to ask the resource to filter its components using a string that specifies some
search criteria (i.e., a filter). The default implementation provided by `smtk::resource::Resource::queryOperation` allows users to filter components based on the existence and values of properties.
For example, a geometric model of a motor will have many model faces that might
each be marked with properties to indicate which are bearing surfaces, which are
fastener or alignment surfaces, which surfaces will be in contact with coolant
or fuel, etc.

In order to allow user interface components to only show relevant model entities,
the resource's :smtk:`queryOperation <smtk::resource::Resource::queryOperation>`
method accepts strings in the following format:

    ``[`` property-type  [ ``{`` property-name [ ``=`` property-value ] ``}`` ]

where

+ ``property-type`` is one of the following string literals ``string``, ``floating-point``, ``integer``.
+ ``property-name`` is either a single-quoted name or a slash-quoted regular expression
  (i.e., a regular expression surrounded by forward slashes such as ``/(foo|bar)/)``.
+ ``property-value`` is one of the following
    + a single, single-quoted string value to match
      (when searching for string properties),
    + a single, slash-quoted regular expression to match
      (when searching for string properties by regular expression),
    + a single, unquoted integer or floating point value to match
      (when searching for properties of those types), or
    + a tuple (indicated with parentheses) of values, as specified above,
      to match. Note that this implies the property must be vector-valued
      and the length must match the specified tuple's length in order
      for a match to be successful.

Whitespace is allowed anywhere but is treated as significant if it is inside
any quoted string value or regular expression.

Note that single quotes are used because these filter strings
will appear in XML and/or JSON serializations that use double-quotes
to mark the start and end of the query string.
The examples below include the double-quotes around the query as a reminder.

For regular expressions, the c++11 standard library is used to search for matches;
the syntax must be accepted by the std::regex constructor and std::regex_search()
must return true when passed property names or values in order for the
corresponding entity to be included in filtered results.

.. list-table:: Examples of valid query strings.
    :widths: 40 80
    :header-rows: 1

    * - Query string
      - Results
    * - "``[string]``"
      - Components with any string properties at all (but not components without string properties).
    * - "``[integer{'counter'}]``"
      - Any component with an integer property named 'counter' (regardless of the value).
    * - "``[string{'pedigree'='zz'}]``"
      - Components with a string-property named pedigree whose value is "zz"
    * - "``[vector<floating-point>{/.*/=(0,0,0)}]``"
      - Components with any vector<floating-point> property whose value is a 3-entry vector of zeros.
    * - "``[vector<string>{'alphabet'=('abc', 'def')}]``"
      - Components with a vector<string> property named "alphabet" whose value is a vector of 2 strings: one valued "abc" and the next valued "def".

.. list-table:: Invalid non-examples of query strings that will not work.
    :widths: 40 80
    :header-rows: 1

    * - Query string
      - Why This is Invalid
    * - "``[{'pedigree'}]``"
      - You must currently specify the property type.
    * - "``[vector<integer>{'lattice'=(0,*,*)'}]``"
      - There is no way to search for properties with partially-matched array-valued entries.
    * - "``[vector<integer>{'counter'=(*,*,*)'}]``"
      - There is no way to search for properties whose value is a given length yet.
