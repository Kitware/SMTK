Changes in Attribute Resource
=============================

Supporting Category Expressions
-------------------------------

You can now specify a category constraint as a category expression instead of defining it as sets of included and excluded category names.
This not only provided greater flexibility but is also easier to define.  For example in an SBT file this would like the following:

.. code-block:: xml

          <CategoryExpression InheritanceMode="Or">(a * !b) * (d + 'category with spaces') </CategoryExpression>

You can use the following symbols to represent logical operators:

* And
  * ``∧``, ``*``, ``&``
* Or
  * ``∨``, ``+``, ``|``
* Complement
  * ``¬``, ``~``, ``!``

Note that in XML you must use ``&amp;`` to represent ``&`` and that ``∨`` is not the letter v but the Unicode **and** symbol.

In this example the expression will match if the test set of categories contains **a** and either **d** or **category with spaces** but not **b**.


Also bumped the file versions of both the XML (to version 8) and JSON (version 7) for Attribute Resources in order to support these changes.
