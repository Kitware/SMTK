Key Concepts
------------

The attribute resource is composed of C++ classes,
also accessible in Python, whose instances perform the following functions:

:smtk:`Attribute`
  instances represent a dictionary of named values.
  The values are all subclasses of the Item class.
  The entries that may appear in an attribute's dictionary
  are constrained by the attribute's Definition.
  In addition to holding a set of Items, an attribute
  may optionally be attached to (or *associated with* in SMTK's parlance)
  a set of geometric model entities from SMTK's geometric modeling system.

:smtk:`Definition`
  instances hold the set of possible key-value pairs that
  must be present in Attribute instances that reference them.
  A definition may inherit another definition as a base type.
  For instance, deflection, temperature, and voltage boundary
  condition definitions might all inherit a Dirichlet boundary
  condition definition. Even when the base class provides
  no requirements, this is useful for fetching attributes that
  meet a specific condition.

:smtk:`Item <smtk::attribute::Item>`
  instances hold values in an attribute key-value pair.
  The particular subclass of Item determines the type
  of storage used to hold the value (e.g. Int, Double, String,
  RefItem, ModelEntityItem).
  Each item references an ItemDefinition that constrains the
  values that may be held in storage, in much the same way
  that an Attribute has a Definition.
  Some items (those derived from :smtk:`ValueItem`) can
  have other items as children;
  this is used to implement `conditional items`_, where
  the presence of children is predicated on the value taken on
  by their parent item.

:smtk:`ItemDefinition`
  instances constrain the number of values that an Item
  instance may contain as well as the particular values that
  are considered valid.
  For example, an ItemDefinition for temperature could
  specify that temperature is a scalar (i.e., only a single
  value may be held in the Item), that it is a floating point
  value, and that it must be positive.

:smtk:`Resource <smtk::attribute::Resource>`
  instances hold resources of attributes associated with a
  particular purpose such as

  * defining a simulation's input deck (see the
    `simulation workflows repository <https://gitlab.kitware.com/cmb/simulation-workflows>`_
    for examples);
  * specifying locations where input and output files are located
    during the export process (SMTK's simulation subsystem creates
    an attribute resource for this purpose); and
  * defining operations that can be performed on a geometric model
    (SMTK's geometric modeling system uses an attribute resource to
    hold definitions for each modeling operation that can be
    performed by each of its modeling kernels).

.. findfigure:: attribute-system-2.*
   :align: center
   :width: 90%

   At the top left is a UML diagram of the C++ classes in SMTK's attribute system.
   At the bottom left is an example composed of instances of the C++ classes above,
   with like colors indicating the class of the instantiated objects.
   The right side of the figure shows how these components are presented in
   the modelbuilder application and associated to a geometric model entity.

Because it can be tedious to programmatically create a bunch of
instances of the classes above to represent a particular simulation's
input deck, SMTK provides an XML file format for serializing and
deserializing all of the attributes, definitions, items, and item-definitions
stored in an attribute resource.

Interfaces to the attribute resource
------------------------------------

The attribute resource has three interfaces:

1. An XML file syntax for specifying the kinds data to be modeled
for individual simulation codes and problem domains.
This file syntax is used to specify the unique inputs for a
simulation code, including, for example, data types, valid ranges,
and default values.
Specific data can be associated with geometric model entities,
and structural features can be used to
group and classify simulation data.


2. A set of user-interface panels for end users to create
and edit simulation data.
Using standard windows components (buttons, text boxes,
selection lists, etc.), users can enter all of the detailed
data needed to specify a desired simulation.
These user-interface panels are automatically produced
by SMTK at runtime, based on the XML file(s) provided to the system.


3. An API for accessing the simulation data produced by end users.
Once an end-user has completed specifying the simulation,
application software can be used to traverse that data
and generate the simulation input files.
The native SMTK API is C++, and python bindings are also included.
In practice, python scripts are typically used to access the
simulation data and generate the simulation input files.

.. system reads in a set of *definitions* specifying the data that
.. are relevant to each application.


.. will end up in simulation input decks for a given solver.

.. uses as its primary input a set of definitions

.. is configured for specific applications and problem domains
.. by a set of definitions

.. Since the simulation data are unique/specific to individual
.. problem and sovler domains,

Example Workflow
----------------

SMTK can be used in a broad range of scientific and engineering
simulation applications.
In physics-based applications, such as
structural mechanics, computational fluid dynamics (CFD), and
electromagnetic (EM) modeling, simulations are often performed relative
to a geometric model. This model may be created using
computer-aided design (CAD), or computationally
constructed/reconstructed from empirical data.
In either case, a
discretization process is typically performed with the
model to generate geometric input suitable for the simulation code,
often in the form of a finite element or finite volume mesh.
To complete the simulation inputs, the non-geometric inputs are
generated to enumerate the specific boundary conditions, material properties,
solver parameters, etc.

With SMTK, the process of generating simulation input data
can be automated, providing the
dual benefits of work-product reuse and error reduction.
Once the simulation-specific data are defined in an XML *template*,
domain experts or other end users can create the simulation data, or
*attribute data*, for specific problems using the SMTK user
interface panels.
Then simulation-specific python scripts can be used to traverse
the attribute data and write the simulation input files.

.. Wish I could align code & image horizontally

.. _GUIExample:

.. findfigure:: ExampleAttributePanel.*
   :align: center

   The XML :ref:`below <XMLExample>` is used to generate this user interface.
   The fields with yellow backgrounds show default values
   and white backgrounds indicate modified values.

.. _XMLExample:

.. code-block:: xml

  <Definitions>
    <AttDef Type="Example1" Label="Example 1" BaseType="" Version="0"
            Unique="true" Associations="">
      <ItemDefinitions>
        <String Name="ExampleString" Label="String item:" Version="0"
                NumberOfRequiredValues="1">
          <BriefDescription>Enter some string of import</BriefDescription>
          <DefaultValue>Yellow denotes default value</DefaultValue>
        </String>
        <Int Name="ExampleInteger" Label="Integer item:" Version="0"
             NumberOfRequiredValues="1">
          <BriefDescription>For some integer value</BriefDescription>
          <DefaultValue>42</DefaultValue>
        </Int>
        <Double Name="ExampleDouble" Label="Double item:" Version="0"
                NumberOfRequiredValues="1">
          <BriefDescription>For floating-point precision values</BriefDescription>
          <DefaultValue>3.14159</DefaultValue>
        </Double>
        <Double Name="ExampleVector" Label="Double item w/3 values:" Version="0"
                NumberOfRequiredValues="3">
          <BriefDescription>Number of components is set to 3</BriefDescription>
          <ComponentLabels>
            <Label>x</Label>
            <Label>y</Label>
            <Label>z</Label>
          </ComponentLabels>
          <DefaultValue>0</DefaultValue>
        </Double>
        <String Name="SecondString" Label="Another string item:" Version="0"
                NumberOfRequiredValues="1">
          <BriefDescription>Enter some string of import</BriefDescription>
          <DefaultValue>whatever</DefaultValue>
        </String>
      </ItemDefinitions>
    </AttDef>

    <!-- Remaining content not shown -->

.. _conditional items:

-----------------
Conditional items
-----------------

One particular workflow SMTK supports is attributes that
can be defined by multiple, distinct sets of values.
For example, there are many ways to define a circle, four of which are:
using 3 points on the circle,
using a center and radius,
using 2 points (a center plus a point on the circle), and
by inscribing a circle inside a triangle.
There are times when you will want to represent an attribute
that can accept any of these definitions instead of constraining
the user to work with a single construction technique.

SMTK accommodates this by having you create an auxiliary
item whose value enumerates the different possible definitions.
This auxiliary item owns all of the items that are active
depending on the auxiliary item's value.
For our example of a circle, the attribute definition would be

.. _ConditionalXML:

.. literalinclude:: circle.xml
   :language: xml
   :start-after: <!-- + 1 + -->
   :end-before: <!-- - 1 - -->

You can see that each "Structure" section describes a particular
way to define the circle.
Different subsets of the items are active depending on whether
the auxiliary "construction method" value is 0, 1, 2, or 3.

.. _ConditionalGUI:

.. findfigure:: circle-ui.*
   :align: center

   The user interface generated for a conditional item definition.

When SMTK generates a user interface for the attribute above,
the "construction method" value is represented as a tabbed
widget with 1 tab for each of the "Structure" sections above.
The default tab will be "2 points".

----------------------
Dealing with Relevance
----------------------

In many workflows, especially ones that support complex aspects such as multi-physics, typically only a subset of the information specified is relevant with respects to the current task.  For example, a hydrological workflow can have information related to constituent and heat transport; however, this information is not relevant if the task is to model only fluid flow.

Note that this concept of relevance can also apply to the values a discrete item (an item will a discrete list of enumerated values) can be set to.

Once major aspect related to relevance is validity.  If information is not relevant then it should not affect validity.  In the above example, missing information concerning a constituent property should not indicate that the workflow is invalid (in this case invalid means incomplete); however, if the task was to change and require constituent transport, then it would be considered invalid.

In SMTK's attribute resource, attributes and their items provide methods to indicate their relevance.  By default, relevance depends on a set of categories (typically this is the attribute resource's set of active categories) .  If the set of categories satisfies the category constraints associated with attribute or item, then it is considered relevant.

For UI purposes, relevance can also take a requested read access level into consideration.  For example, if a workflow is displaying basic information (indicated by read access level 0) and an attribute contains only items with a greater read level, then none of the attribute items would be displayed and in this scenario the attribute itself would be considered not relevant (w/r to the UI) and should not be displayed.


Custom Relevance for Items
~~~~~~~~~~~~~~~~~~~~~~~~~~

SMTK now supports the ability to assign a function to an attribute item that would
be used to determine if an item is currently relevant. For example, it is now possible to have the relevance of one item depend on the value of another.

**Note** that no effort is made to serialize or deserialize the provided function; your application that uses SMTK must call :smtk:`Item::setCustomIsRelevant()` each time the item is loaded or created.

Custom Validity for Items
~~~~~~~~~~~~~~~~~~~~~~~~~~

Similar to custom relevance, SMTK allows the assignment of a function for each attribute item, with custom logic determining whether that item is valid.

**Note** that no effort is made to serialize or deserialize the provided function; your application that uses SMTK must call :smtk:`Item::setCustomIsValid()` each time the item is loaded or created.

Custom Relevance for Enumerated Values
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SMTK now supports the ability to assign a function to a value item's definition (consisting of discrete enumerated values). Similar to relevance function for item, this  function  would
be used to determine if an item's enum value  is currently relevant. For example, it is now possible to restrict the possible relevant values an item can based on the value of another item.

**Note** that no effort is made to serialize or deserialize the provided function; your application that uses SMTK must call :smtk:`Item::setCustomIsRelevant()` each time the item is loaded or created.

Related API
~~~~~~~~~~~

* :smtk:`Attribute::isRelevant` - method to call to see if an attribute is relevant
* :smtk:`Item::isRelevant` - method to call to see if an item is relevant
* :smtk:`Item::defaultIsRelevant` - default relevance method used by :smtk:`Attribute::isRelevant` when no custom relevance function has been specified
* :smtk:`Item::setCustomIsRelevant` - method used to set a custom relevance function
* :smtk:`Item::customIsRelevant` - method used to return the custom relevance function if one has been set
* :smtk:`Attribute::isValid` - method to call to see if an attribute is valid
* :smtk:`Item::isValid` - method to call to see if an item is valid
* :smtk:`Item::defaultIsValid` - default validity method used by :smtk:`Attribute::isValid` when no custom validity function has been specified
* :smtk:`Item::setCustomIsValid` - method used to set a custom validity function
* :smtk:`Item::customIsValid` - method used to return the custom validity function if one has been set
* :smtk:`ValueItem::relevantEnums` - returns a list of enum strings representing the current relevant discrete values
* :smtk:`ValueItemDefinition::relevantEnums` - the method called by :smtk:`ValueItem::relevantEnums` when no custom relevance function has been specified
* :smtk:`ValueItemDefinition::defaultIsEnumRelevant` - the default relevance method for enum values based solely on categories and read access level
* :smtk:`ValueItemDefinition::setCustomEnumIsRelevant` - method used to set a custom enum relevance function
* :smtk:`ValueItemDefinition::customEnumIsRelevant` - method used to return the custom enum relevance function if one has been set

Please see `customIsRelevantTest <https://gitlab.kitware.com/cmb/smtk/-/blob/master/smtk/attribute/testing/cxx/customIsRelevantTest.cxx>`_ for an example of how to use this functionality.

------------------
Dealing with Units
------------------

SMTK attribute resources support units via the `units`_ library;
you can visit its repository for documentation on how to use the library,
but SMTK does not require you to use the units library for most tasks.
Instead, workflow designers specify units for items as a string and SMTK
user-interface components accept values from users in any compatible units.
This way, when you export a simulation attribute, all the unit conversions
are automatically performed by SMTK for you.

Units and Value Items
~~~~~~~~~~~~~~~~~~~~~

The value-based Items (DoubleItem, StringItem, IntegerItem, etc..) can have
their *native* units (the units that the Item returns its value in) defined
in their :smtk:`ItemDefinition <smtk::attribute::ItemDefinition>`. In the
case of DoubleItems, they can also be explicitly assigned to the item itself
(when its definition does not specify the units).

.. _units: https://gitlab.kitware.com/units/units/

For example, a DoubleItem **foo** has its units specify by its Definition as "meters".  If foo is assigned "10 cm", its returned value will be 0.1 (meters).

Changing an Item's explicit units can affect its current values.  For example if a DoubleItem **bar** is explicitly assigned "cm" as its units and its value is set to "100 m", its return value will be 10000 (cm).  If its explicit units are then changed to "meters", its return value is now 100 (meters).  However, if an Item is assigned new units that not compatible to its input values, the input values are replaced with the new units.  For example, it we were to later change **bar**'s units to "K" (Kelvin), it return value will be 100 (Kelvin).

Please see `unitDoubleItem <https://gitlab.kitware.com/cmb/smtk/-/blob/master/smtk/attribute/testing/cxx/unitDoubleItem.cxx>`_ for a simple example of using units with Items.

Units and Attributes
~~~~~~~~~~~~~~~~~~~~

There are times when the attribute itself represents information that has units associated with it.  For example, an attribute could represent a temperature field that will be computed by a solver and the user needs to indicate the units they would like the field calculated in.  In this case there are no *values* but just the units themselves.  Attribute definitions can now have units set on them.  These units are inherited by definitions that are derived from it as well as the attributes that produced from it.  A derived definition can override these units, through by default as in the case of value items, the derived definition's units must be compatible with its base definition units.  So for example if the base definition's units are meters, you could set the derived definition's units to miles but not to feet/sec.  You can however explicitly force a unit change if needed.

Attributes inherit units from the definition they are based on and as with derived , can have different units locally set on them.  However, in this case the new units must be compatible with those of its definition (there is no way of forcing incompatible units).

Definitions with Units "*"
++++++++++++++++++++++++++

There are cases where attributes coming from the same definition need to have different/incompatible units.  For example, consider associating units on an attribute representing an expression.  The definition may not want to restrict what units its attributes can have.
In this case, workflow designers can set the definition's units to be "*" indicating any units are allowed.  Note that this also pertains to any definition derived from it.

An attribute whose definition has "*" units will by default be unit-less (meaning that the units it returns are the empty string). The only restriction in terms of setting the attribute's local units is that the new units must either be one supported by the units system or an empty string meaning that the attribute is unit-less.

Expression Attributes with Units
++++++++++++++++++++++++++++++++
Units are now taken into consideration when assigning an expression attribute to a double item that supports expressions.  Previously, in order to assign an expression to the double item, only its definition is checked to see if it is compatible to the double item's expression type requirement.
Now the following additional constraints are now enforced:

* If the double item is unit-less then only unit-less expression attributes can be assigned to it
* If the double item has units associated with it, then only unit-less expression attributes, or expression attributes whose units can be converted to those of the double item can be assigned to it.

If the case where both the expression attribute and the double item have different (but compatible units) the value computed from the expression will be converted into the units specified by the item.  So for example if an expression returns a value of 4 feet and the double item's units is in inches, then the double item's value method will return 48 (inches).

Please see `unitDoubleItem <https://gitlab.kitware.com/cmb/smtk/-/blob/master/smtk/attribute/testing/cxx/unitInfixExpressionEvaluator.cxx>`_ for a simple example of using expressions involving units with Items.


Related API
~~~~~~~~~~~

* :smtk:`Attribute::units` - method to return the units associated with the attribute (either locally set or inherited from its definition)
* :smtk:`Attribute::localUnits` - method to return the local units explicitly associated with the attribute
* :smtk:`Attribute::setLocalUnits` - method to set the local units explicitly associated with the attribute
* :smtk:`Attribute::supportUnits` - method to used to determine if the attribute supports units
* :smtk:`Definition::units` - method to return the units associated with the definition (either locally set or inherited from its derived definition)
* :smtk:`Definition::localUnits` - method to return the local units explicitly associated with the definition
* :smtk:`Definition::setLocalUnits` - method to set the units explicitly associated with the definition
* :smtk:`Definition::unitSystem` - method to return the units system associated with the definition

Please see `unitAttributeUnits <https://gitlab.kitware.com/cmb/smtk/-/blob/master/smtk/attribute/testing/cxx/unitAttributeUnits.cxx>`_ for a simple example of using units with Attributes and Definitions.


Migration to newer schema
-------------------------

Over time, the schema used by an attribute resource may need to be
modified to fix issues or add features.
SMTK provides some support to aid you in migrating existing attribute
resources from an old schema to a new one while retaining as much
user-provided information as possible.
This is intended to be accomplished by copying attribute instances and
other information from the existing resource into a new resource whose
definitions are from a newer schema.
See the ``itemUpdater`` test for an example how to register
functions or python scripts to help with this process.
