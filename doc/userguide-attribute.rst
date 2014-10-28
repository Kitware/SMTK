.. _smtk-attribute-sys:

***********************
SMTK's Attribute System
***********************

General Description
===================

.. figure:: figures/attributeManager.*
   :align: center


Background
----------

SMTK's first major component is its attribute modeling system,
which provides a way to model and represent non-geometric
simulation data such as material properties, boundary conditions,
and solver parameters.

Key Concepts
------------

The attribute system is composed of C++ classes,
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

:smtk:`ItemDefinition`
  instances constrain the number of values that an Item
  instance may contain as well as the particular values that
  are considered valid.
  For example, an ItemDefinition for temperature could
  specify that temperature is a scalar (i.e., only a single
  value may be held in the Item), that it is a floating point
  value, and that it must be positive.

:smtk:`System <smtk::attribute::System>`
  instances hold collections of attributes associated with a
  particular purpose such as defining a simulation's input deck.

Because it can be tedious to programmatically create a bunch of
instances of the classes above to represent a particular simulation's
input deck, SMTK provides an XML file format for serializing and
deserializing all of the attributes, definitions, items, and item-definitions
stored in an attribute system.

Components of the attribute system
----------------------------------

The attribute system has three main components:

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

Next:

* XML file example
* UI screenshot(s)

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

.. Wish I could align code & image horizontally

.. _GUIExample:

.. figure:: figures/ExampleAttributePanel.*
   :align: center


   The XML above is used to generate this user interface.
   The fields with yellow backgrounds show default values
   and white backgrounds indicate modified values.


Template File Syntax (Reference)
================================

File Layout
-----------
All attribute template and instance files must contain the
<SMTK_AttributeSystem> XML element. The following table shows the XML
Attributes that can be included in this XML Element.

.. list-table:: XML Attributes for <SMTK_AttributeSystem> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description
   * - Version
     - Integer value that indicates the SMTK attribute format
       (Required)

       Valid Values are 1 or 2

This element can contain the following optional children XML Elements:

- Includes : used for including additional attribute files (see `Includes Section`_)
- AdvanceLevels : used to define various advance access levels used in
  GUIs (see `Advance Level Section`_)
- Categories : used to define workflow specific categories (see `Category Section`_)
- Analyses : used to define various analysis groups (see `Analysis Section`_)
- Definitions : used to define attribute definitions (see `Definitions Section`_)
- Attributes : used to define attributes
- Views : used to define various Views (used to create GUIs)
- ModelInfo

Includes Section
--------------------
The attribute format supports the ability to include other attribute
files.  This allows designers to assemble a complete attribute
description by referencing attribute files that represent specific
aspects.  For example a set of attribute definitions may be referenced
by several different simulation workflows.  Below is an example of
including two attribute files both located in a subdirectory
IncludeTest.

.. code-block:: xml

  <Includes>
    <File>includeTest/b.xml</File>
    <File>includeTest/a.xml</File>
  </Includes>

Each include file is contained within a File XML Element.

Advance Level Section
-------------------------------------
This is an optional section used to describe the various access levels
used in GUIs created using SMTK's QT Attribute classes.  For example a
workflow could consist of the following advance levels:

- General
- Intermediate
- Expert

Using  the <AdvanceLevels> element the following represents the above
access levels.

.. code-block:: xml

  <AdvanceLevels>
    <Level Label="General" Color="0.0, 1.0, 0.0"> 0 </Level>
    <Level Label="Intermediate" Color="1.0, 1.0, 0.0"> 1 </Level>
    <Level Label="Expert" Color="1.0, 1.0, 0.0"> 2 </Level>
  </AdvanceLevels>

The value of each Level XML Element determines the  ordering
of the access levels and are used by the items contained within
Definitions (see xxx).  Notice that in GUIExample_, the upper left
corner "Show Level" entry is based on the Advance Levels.

Advance Level Element Format
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Each advance level is represented by using a <Level> XML Element.  The
value of the element determines the absolute access level and should
not be repeated amoung sibling Level Elements.  The higher the value
the more "advance" the level.

The following table shows the XML
Attributes that can be included in this XML Element.

.. list-table:: XML Attributes for <Level> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description
   * - Label
     - String value representing the access level name to be displayed
       in the GUI (Required)
   * - Color
     - String value representing the color to be used when displaying

       items that are associated with this access level.  The format

       is "r, g, b" where r, g, and b are a value between 0 and 1
       inclusive (Optional)


Category Section
-----------------------------
This is an optional section describing the set of categories used
within the file.  Items within Definitions can then be associated with
these categories.  In addition, analyses are defined as sets of
categories.  For example, the following xml would define the following
categories:

- Constituent Transport
- Heat Transfer
- Flow
- Time
- General

.. code-block:: xml

  <Categories>
    <Cat>Constituent Transport</Cat>
    <Cat>Heat Transfer</Cat>
    <Cat>Flow</Cat>
    <Cat>Time</Cat>
    <Cat>General</Cat>
  </Categories>

Each category is contained within a Cat XML Element.

The following table shows the XML
Attributes that can be included in <Categories> Element.

.. list-table:: XML Attributes for <Categories> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description
   * - Default
     - String value representing the default categories a Definition's
       Item belongs to

       when no category is specified.

Analysis Section
---------------------------
This is an optional section that define analyses.  An analysis is
defined as a collection of categories.  For example, using the
categories defined in the`Category Section`_, the following XML would
define two analyses (Ground Water Flow, and Ground Water with Heat
Transfer).

.. code-block:: xml

  <Analyses>
    <Analysis Type="Groundwater Flow">
      <Cat>Flow</Cat>
      <Cat>General</Cat>
      <Cat>Time</Cat>
    </Analysis>
    <Analysis Type="Groundwater Flow with Heat Transfer">
      <Cat>Flow</Cat>
      <Cat>General</Cat>
      <Cat>Heat Transfer</Cat>
      <Cat>Time</Cat>
    </Analysis>
  </Analyses>

Analysis Element Format
^^^^^^^^^^^^^^^^^^^^^^^
Each Analysis is defined within an <Analsyis> XML Tag.

The following table shows the XML
Attributes that can be included in this XML Element.

.. list-table:: XML Attributes for <Analysis> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description
   * - Type
     - String value representing the type analysis being
       defined. Note that the type should be

       unique with respects to all other analyses being defined. (Required)


Each element contains a set of Cat XML Elements.

Definitions Section
---------------------------------
This is an optional section that defines a set of attribute
definitions used to generate attrubutes with a SMTK-based program.
This section is created using the <Definitions> XML Element.  See
XMLExample_ for example of creating a set of attribute definitions.

This element is composed of a set of AttDef XML Elements

AttDef Element Format
^^^^^^^^^^^^^^^^^^^^^
This element define an attribute definition.

This element can contain the following children XML Elements:

.. list-table:: XML Children Elements for <AttDef> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description
   * - <ItemDefinitions>
     - Defines the items contained within the attributes generated

       by this definition (Optional).

       See `Item Definitions Format`_.
   * - <BriefDescription>
     - Provides a brief description of the definition (Optional).
   * - <DetailedDescription>
     - Provides a detailed description of the definition (Optional).


The following table shows the XML
Attributes that can be included in this XML Element.

.. list-table:: XML Attributes for <AttDef> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description
   * - Type
     - String value representing the attribute definition type

       being defined. (Required).

       Note that this value should be unique with respects to all
       other definitions

       being defined with this section as well as all definitions
       being included

       via the Includes XML Element (See `Includes Section`_)

   * - BaseType
     - String value representing the attribute defintion that this
       defintion

       is derived from (Optional).

       Note that the base definition must be defined prior to this
       definition

       either in section or in the Includes Section.
   * - Label
     - String value representing the name display in a GUI (Optional)

       Note that is not specified, the Type value is displayed
   * - Version
     - Integer value representing the "version" of the definition (Optional)

       This is used for versioning the definition.  If not specified

       0 is assumed.

   * - Abstract
     - Boolean value used to indicate if the definition is abstract

       or not (Optional).

       If not specified, the definition is not abstract

       Note that abstract definitions can not generate attributes.

   * - AdvanceLevel
     - Integer value used to indicate the advance level associated

       with the definition and the attrubutes it
       generates. (Optional).

       This value should match one of the advance values

       defined in the `Advance Level Section`_. If not

       specified, 0 is assumed.
   * - Unique
     - Boolean value used to indicate if the attributes this
       definition

       generates are unique with respects to the model entities it

       associated with.  A model entity can only have one unique

       attribute of a given type associated with it (Optional).

       If not specified, the definition is assumed to
       be not unique.
   * - Nodal
     - Boolean value used to indicate if the attribute effects the
       nodes of

       the analysis mesh or the elements (Optional).

       If not specified the definition's attributes are not nodal.
   * - Associations
     - String value indicating what type of model entities this
       definition's

       attributes can be associated on (Optional).

       The information is represented as a string consisting of

       a set of the following characters:


       v (vertices), e (edges), f (faces), r (volumetric regions),
       m(model), g(groups).

       If not specified the definition's attributes can not be
       associated

       with any model entities.
   * - NotApplicationColor
     - String value representing the color to be used when coloring

       model entities  that are not associated with this
       definition's attribute.

       The format is "r, g, b" where r, g, and b are a value between 0
       and 1

       inclusive (Optional)

       If not specified its value is 0, 0, 0, 0
   * - Default Color
     - String value representing the color to be used when coloring

       model entities  that are associated with this
       definition's

       attribute by default.

       The format is "r, g, b" where r, g, and b are a value between 0
       and 1

       inclusive (Optional)

       If not specified its value is 0, 0, 0, 0

Item Definitions Format
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
This subsection of an AttDef Element contains the definitions of all the
items to be created within the attributes created by the attribute
definition.  The section is represented by the <ItemDefinitions> XML
Element and can contain any of the elements decribed in the Item
Definition Section.

Item Definition Section
------------------------
All of the XML Elements described within this section can be added to
the <ItemDefinitions> of an attribute defintion <AttDef>.

The types of items currently supported include:
 - Basic Values: Doubles, Integers, and Strings
 - Groups
 - Attribute References
 - Directories and Files
 - Model Information
 - Voids

All the elements can contain the following children XML Elements. Note
that each element may have additional XML Children Elements that are
specific to it.

.. list-table:: Common XML Children Elements for Item Definition Elements
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description
   * - <Categories>
     - Defines the categories that the item belongs to  (Optional).

       This element contains at set of <Cat> elements with each

       containing a category defined is the Category Section.

       See `Category Section`_.
   * - <BriefDescription>
     - Provides a brief description of the item (Optional).
   * - <DetailedDescription>
     - Provides a detailed description of the item (Optional).

All of the elements support the following common XML Attributes.  Note
that each element may have additional XML Attributes that are specific to
it.

.. list-table:: Common XML Attributes for Item Definition Elements
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description
   * - Label
     - String value representing the name of the item

       being defined. (Required).

       Note that this value should be unique with respects to all
       other items

       contained within this attribute definition (including its Base Type)
   * - Version
     - Integer value representing the "version" of the item (Optional)

       This is used for versioning the item.  If not specified

       0 is assumed.

   * - Optional
     - Boolean value indicating if the item is considered optional

       or required (Optional).

       If not specified the item is considered to be required.

   * - IsEnabledByDefault
     - Boolean value indicating if the item is considered to be
       enabled

       by default (Optional).

       Note this is only used when Optional="true".

       If not specified the item is considered to be not enabled.

   * - AdvanceLevel
     - Integer value used to indicate the advance level associated

       with the item. (Optional).

       This value should match one of the advance values

       defined in the `Advance Level Section`_. If not

       specified, 0 is assumed.


   * - AdvanceReadLevel
     - Integer value used to indicate the advance read level associated

       with the item. (Optional).

       This value should match one of the advance values

       defined in the `Advance Level Section`_.  Note that this is

       ignored if the AdvanceLevel XML Attribute is used.

       If not specified, 0 is assumed.


   * - AdvanceWriteLevel
     - Integer value used to indicate the advance write level associated

       with the item. (Optional).

       This value should match one of the advance values

       defined in the `Advance Level Section`_. Note that this is

       ignored if the AdvanceLevel XML Attribute is used.

       If not specified, 0 is assumed.

Basic Value Items
^^^^^^^^^^^^^^^^^^^^^^^^


Attribute Section <Attributes>
------------------------------
.. todo::

   Describe attributes and how they are serialized



XML attributes
^^^^^^^^^^^^^^^^^^^^^^^

Children elements
~~~~~~~~~~~~~~~~~

File Item Definition <File>
"""""""""""""""""""""""""""
.. todo::

   Describe file items and how they are serialized

Group Item Definition <Group>
"""""""""""""""""""""""""""""
.. todo::

   Describe group items and how they are serialized

Integer Item Definition <Int>
"""""""""""""""""""""""""""""
.. todo::

   Describe integer items and how they are serialized

String Item Definition <String>
"""""""""""""""""""""""""""""""
.. todo::

   Describe string items and how they are serialized

Ref Item Definition <Ref>
"""""""""""""""""""""""""
.. todo::

   Describe attribute reference items and how they are serialized

Model Entity Item Definition <ModelEntity>
""""""""""""""""""""""""""""""""""""""""""

A :smtk:`ModelEntityItem`, which appears in XML as a <ModelEntity> is an
item belonging to an attribute stored as a UUID that refers to an
SMTK model entity.
These model entities may be regions, faces, edges, vertices, or even
higher-level conceptual entities such as models, groups, or instances (used
in modeling scene graphs and assemblies).

Void Item Definition <Void>
"""""""""""""""""""""""""""
.. todo::

   Describe "void" items and how they are serialized



RootView Section <RootView>
---------------------------
.. todo::

   Describe root views and how they are serialized
