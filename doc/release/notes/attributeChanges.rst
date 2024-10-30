Changes to Attribute Resource
=============================

Added `attribute::Resource::hasDefinition(type)` Method
------------------------------------------------------

This method will return true if the resource already contains a Definition with
the requested type.

Added `attribute::Analyses assignment method
--------------------------------------------

You can now assign the contents of one Analyses instance to another.  This will create copies
of all of the Analysis instances owned by the source Analyses.

Units Support for Definitions and Attributes
--------------------------------------------

You can now assign units to Definitions and Attributes.  This can be useful when an Attribute
represents a concept that has units but does not have an explicit value.  For example, if an
Attribute represents a temperature field that would be created by a simulation, you may want to
assign a default unit to its Definition (such as Kelvin) but allow the user to change the
Attribute's units to Celsius.

The rules for assigning local units to an Attribute that override those inherited through its definition
are the same as the case of assigning a value with units to a ValueItem.

Derived Definitions inherit units associated with their base Definition.  When overriding the units being inherited, by default a Definition's units must be compatible with the units coming from its base Definition, though the method provides an option to force units to be set even if they are not compatible.

Definitions whose units are "*" indicate that Definitions derived and Attributes that are create from can be assigned any supported units.

Expression Attributes and Value Items with Units
-------------------------------------------------

ValueItems will now test units when assigning expressions.  If the expression has units and they
are not convertible to the item's units, the assignment will now fail.

Also if a double item's units are different from its expression, the expression's
evaluated value is converted to those of the item when calling its value methods.

See smtk/attribute/testing/c++/unitInfixExpressionEvaluator.cxx and data/attribute/DoubleItemExample.sbt for examples.

Expanded Support For Property Filtering
---------------------------------------

Properties on Definitions can now be *inherited* by Attributes and derived Definitions when creating queries and association rules.

For example if Definition **A** has a floating-point property "alpha" with value 5.0, and if Definition **B** is derived from **A** and Attribute **a** is from **A** and Attribute **b** is from **B**, then all would match the rule "any[ floating-point { 'alpha' = 5.0 }]".  If later **B** was to also have floating-point property "alpha" with value 10.0 associated with it, it would override the value coming from **A** so both it and **b** would no longer pass the rule.  Similarly, properties on Attributes can override the values coming from its Definition.

Also implemented differentiation between attributes and definitions.  If the rule starts with *attribute* then only attributes have the possibility of matching the rest of the rule. Similarly, if the rule starts with *definition* then only definitions have the possibility of matching the rest of the rule.  If the rule starts with *any* or * then either attributes or definitions are allowed.

**Note** that the ``properties()`` methods on attribute and definition objects do not return *inherited* values but instead only those values local to the object. In the future, we may add methods to interact directly with inherited properties but for now only filter-strings process inherited properties.

See smtk/attribute/testing/c++/unitPropertiesFilter.cxx and data/attribute/propertiesFilterExample.sbt for examples.

Expanded Expression Support
---------------------------

Value Item Definitions now represent their expression information as a Component Item Definition.  This provides more flexibility when specifying which types of attributes can be assigned to Value Items as expressions.  You can now how multiple acceptable conditions which include regular expressions as well as property constraints on both the attribute and its definitions. In addition, you can now include rejection conditions.

The older mechanism of specifying the expression constraint as a string representing the attribute definition type is still supported.  Both the JSON and XML file formats have been updated to support this new functionality.  Here is an example of how to
specify expression information in XML using this more flexible approach:

.. code-block:: xml

        <Double Name="testDoubleItem" Label="testDoubleItem" NumberOfRequiredValues="1">
          <CategoryExpression InheritanceMode="And" />
          <ExpressionInfomation Name="expression" NumberOfRequiredValues="1">
            <Accepts>
              <Resource Name="smtk::attribute::Resource" Filter="attribute[type='doubleItemTestExpression']" />
            </Accepts>
            <Rejects />
          </ExpressionInfomation>
        </Double>

Ability to Extract Information from Attribute Queries
-----------------------------------------------------

`smtk::attribute::Resource::extractGrammarInfo`` provides the ability to examine an attribute query string and extract information from it.  Currently this information includes:

* Type Information - that can either refer to an attribute definition type name or a regular expression that will be applied to type names.
* Indication if the query contains property constraints.
