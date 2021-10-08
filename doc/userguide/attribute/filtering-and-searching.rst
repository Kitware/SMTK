=============================================
Filtering, Searching and Reference Item Rules
=============================================

It is possible
to ask the attribute resource to filter its attributes using a string that specifies some
search criteria (i.e., a filter). The current implementation expands on the base functionality provided by  `smtk::resource::Resource::queryOperation` which allows users to filter components based on the existence and values of properties by adding the ability to search for attributes based on their *type* (i.e., the name of its definition and those definitions which inherit it).

Note that these query strings can be used in a resource's find(...) method as well as in a Reference Item Definition's Accept or Reject rules

The current grammar syntax for attribute resources is:

+ One of the following : **attribute**, *****, or **any** followed by **[**
+ One or more of the following sub-phrases:

  + Attribute Type Phrase

    + **type =** followed by one of the following

      + **'attribute-type-name'**
      + **/regular-expression/**

  + Property Phrase

    + **property-type** optionally followed by

      + **{ property-name** optionally followed by

        + **= property-value**
      + **}**
+ **]**

For example, consider an attribute resource consisting of the following:

+ Definitions (where a : b means a is derived from b)

  + Base
  + MyDef1 : Base
  + MyDef2 : Base
+ Attributes (where a : d means Attribute a is based on Definition d)

  + a : Base
  + b : MyDef1
  + c : MyDef2
  + c1 : MyDef2

Lets also add some properties to these attributes:
+ a has long property alpha = 10
+ b has long property beta = 100
+ c has long property alpha = 200
+ c1 has long property alpha = 10 and double property gamma = 3.14


.. list-table:: Examples of valid query strings.
    :widths: 40 80
    :header-rows: 1

    * - Query string
      - Results
    * - "``attribute [type = 'Base']``"
      - Returns a, b, c, c1 since all are derived from Definition Base.
    * - "``attribute [type = 'MyDef2']``"
      - Returns c, c1 since they are derived from Definition MyDef2.
    * - "``attribute [type = '/My.*/']``"
      - Returns b, c, c1 since each has a Definition in its derivation whose type matches the RegEx.
    * - "``attribute [long{'alpha'}]``"
      - Returns a, c, c1 since they are have a long property named alpha.
    * - "``attribute [long {'alpha'}]``"
      - Returns a, c, c1 since they are have a long property named alpha.
    * - "``attribute [type = 'MyDef2', long{'alpha'}]``"
      - Returns c, c1 since they are have a long property named alpha and are derived from MyDef2.
    * - "``attribute [long {'alpha' = 10}]``"
      - Returns a since it has  a long property named alpha with value 10.
    * - "``attribute [long{'alpha'}, double{'gamma'}]``"
      - Returns c1 since it is the only one with both a double named gamma and long property named alpha.

See smtk/attribute/testing/cxx/unitAssociationTest.cxx for a complete example.
