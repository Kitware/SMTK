Changes in Attribute Resource
=============================

Supporting Category Expressions
-------------------------------

You can now specify a category constraint as a category expression instead of defining it as sets of included and excluded category names.
This not only provided greater flexibility but is also easier to define.  For example in an SBT file this would like the following:

.. code-block:: xml

          <CategoryExpression InheritanceMode="Or">(a &amp; !b) &amp; (d | 'category with spaces') </CategoryExpression>

Note that in XML ``&amp;`` represents ``&``.

In this example the expression will match if the test set of categories contains **a** and either **d** or **category with spaces** but not **b**.


Also bumped the file versions of both the XML (to version 8) and JSON (version 7) for Attribute Resources in order to support these changes.
