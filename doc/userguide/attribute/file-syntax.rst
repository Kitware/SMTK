Template File Syntax (Reference) for Version 5 XML Attribute Files
==================================================================

File Layout
-----------
All attribute template and instance files must contain the
<SMTK_AttributeResource> XML element. The following table shows the XML
Attributes that can be included in this XML Element.

.. list-table:: XML Attributes for <SMTK_AttributeResource> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Version
     - Integer value that indicates the SMTK attribute format (Required)

       Current value is 5 (latest version)

This element can contain the following optional children XML Elements:

- Includes : used for including additional attribute files (see `Includes Section`_)
- AdvanceLevels : used to define various advance access levels used in
  GUIs (see `Advance Level Section`_)
- Categories : used to define workflow specific categories (see `Category Section`_)
- Analyses : used to define various analysis groups (see `Analysis Section`_)
- Item Blocks : used to define reusable  blocks of Item Definitions(see `Item Blocks Section`_)
- Definitions : used to define attribute definitions (see `Definitions Section`_)
- Attributes : used to define attributes
- Views : used to define various Views (used to create GUIs)

Includes Section
--------------------
The attribute format supports the ability to include other attribute
files.  This allows designers to assemble a complete attribute
description by referencing attribute files that represent specific
aspects.  For example a set of attribute definitions may be referenced
by several different simulation workflows.  Below is an example of
including two attribute files both located in a sub-directory
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
Definitions (see xxx).  Notice that in :ref:`the GUI example <GUIExample>`,
the upper left corner "Show Level" entry is based on the Advance Levels.

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
       Item belongs to when no category is specified.

Property Section for the Attribute Resource
-------------------------------------------
This is an optional section describing a set of properties  that should be
added to the Attribute Resource.  The Property section is defined by a XML
**Properties** node which is composed of a set of children **Property** nodes as shown below:

.. code-block:: xml

  <Properties>
    <Property Name="pi" Type="Int"> 42 </Property>
    <Property Name="pd" Type="double"> 3.141 </Property>
    <Property Name="ps" Type="STRING">Test string</Property>
    <Property Name="pb" Type="bool"> YES </Property>
  </Properties>

You can also look at data/attribute/attribute_collection/propertiesExample.rst and smtk/attribute/testing/cxx/unitXmlReaderProperties.cxx for a sample XML file and test.

The following table shows the XML
Attributes that can be included in <Property> Element.

.. list-table:: XML Attributes for <Property> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Name
     - String value representing the name of the property to be set.

   * - Type
     - String value representing the type of the property to be set. **Note** that the value is case insensitive.


The values that the **Type** attribute can be set to are:

* int for an integer property
* double for a double property
* string for a string property
* bool for a boolean property

The node's value is the value of the property being set.

Supported Values for Boolean Properties
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The following are supported values for true:

* t
* true
* yes
* 1

The following are supported values for false:

* f
* false
* no
* 0

**Note** that boolean values are case insensitive and any surrounding white-space will be ignored.

Properties and Include Files
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
If you include a Attribute XML file that also assigns Resource Properties, the include file's Properties are assigned first.  Meaning that the file suing the include file can override the Properties set by the include file.

**Note** - the ability to unset a Property is currently not supported.

**Note** - Properties are currently not saved if you write out an Attribute Resource that contains properties in XML format.

Analysis Section
---------------------------
This is an optional section that define analyses.  An analysis is
defined as a resource of categories.  For example, using the
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
       defined. Note that the type should be unique with
       respects to all other analyses being defined.
       (Required)


Each element contains a set of Cat XML Elements.

Item Blocks Section
---------------------------------
Item Definition Blocks allows the reuse of a group of Item Definitions in different Attribute Definitions.  Providing a "hasA" relationship as opposed to the currently supported "isA". These blocks can then be referenced in the "ItemDefinitions" nodes of Attribute or Group Item Definitions or in the "ChildrenDefinitions" nodes for Reference or Value Item Definitions.  Blocks themselves can reference other blocks.  But care must be taken not to form a recursive relationship.  In the parser detects such a pattern it will report an error.

When referencing a Block, the items will be inserted relative to where the Block is being referenced.

Note that category constraints are inherited as usual and that Blocks can call other blocks.  Here is an example of an Item Block:

.. code-block:: xml

  <ItemBlocks>
    <Block Name="B1">
      <ItemDefinitions>
        <String Name="s1">
          <Categories>
            <Cat>Solid Mechanics</Cat>
          </Categories>
        </String>
        <Int Name="i1"/>
      </ItemDefinitions>
    </Block>
  </ItemBlocks>

  <Definitions>
    <AttDef Type="Type1">
      <Categories>
        <Cat>Fluid Flow</Cat>
      </Categories>
      <ItemDefinitions>
        <Double Name="foo"/>
        <Block Name="B1"/>
        <String Name="bar"/>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="Type2">
      <Categories>
        <Cat>Heat Transfer</Cat>
      </Categories>
      <ItemDefinitions>
        <Block Name="B1"/>
        <String Name="str2"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

See data/attribute/attribute_collection/ItemBlockTest.sbt and smtk/attribute/testing/cxx/unitItemBlocks.cxx for examples.


Definitions Section
---------------------------------
This is an optional section that defines a set of attribute
definitions used to generate attributes with a SMTK-based program.
This section is created using the <Definitions> XML Element.
See :ref:`the example XML <XMLExample>` for how to create a set
of attribute definitions.

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

   * - <CategoryInfo>
     - Specifies the local category constraints specified on the
       Definition

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
   :class: smtk-xml-att-table

   * - XML Attribute
     - Description

   * - Type
     - String value representing the attribute definition type
       being defined. (Required).

       Note that this value should be unique with respects to all
       other definitions being defined with this section as well
       as all definitions being included via the Includes XML
       Element (See `Includes Section`_)

   * - BaseType
     - String value representing the attribute defintion that this
       definition is derived from.
       (Optional)

       Note that the base definition must be defined prior to this
       definition either in section or in the Includes Section.

   * - Label
     - String value representing the name display in a GUI
       (Optional)

       Note that if not specified, the Type value is displayed.

   * - Version
     - Integer value representing the "version" of the definition.
       (Optional)

       This is used for versioning the definition.
       If not specified then 0 is assumed.

   * - Abstract
     - Boolean value used to indicate if the definition is abstract or not.
       (Optional)

       If not specified, the definition is not abstract.
       Note that abstract definitions can not generate attributes.

   * - AdvanceLevel
     - Integer value used to indicate the advance level associated
       with the definition and the attributes it generates.
       (Optional)

       This value should match one of the advance values
       defined in the `Advance Level Section`_.
       If not specified, 0 is assumed.

   * - Unique
     - Boolean value used to indicate if the attributes this definition
       generates are unique with respects to the model entities it
       associated with.
       A model entity can only have one unique attribute of a given
       type associated with it.
       (Optional)

       If not specified, the definition is assumed to be non-unique.

   * - Nodal
     - Boolean value used to indicate if the attribute effects the
       nodes of the analysis mesh or the elements.
       (Optional)

       If not specified the definition's attributes are not nodal.

   * - Associations
     - String value indicating what type of model entities this
       definition's attributes can be associated on.
       (Optional)

       The information is represented as a string consisting of
       a set of the following characters separated by vertical
       bars (|):

       v (vertices)

       e (edges)

       f (faces)

       r (volumetric regions)

       m (model)

       g (groups)

       An example would be "e|f" for an attribute which may
       be associated with both edges and faces.
       If not specified, the definition's attributes can not be
       associated with any model entities.

   * - NotApplicationColor
     - String value representing the color to be used when coloring
       model entities  that are not associated with this
       definition's attribute.
       (Optional)

       The format is "r, g, b" where r, g, and b are a value between 0
       and 1 inclusive.
       If not specified its value is 0, 0, 0.

   * - Default Color
     - String value representing the color to be used when coloring
       model entities  that are associated with this definition's
       attribute by default.
       (Optional)

       The format is "r, g, b" where r, g, and b are a value between 0
       and 1 inclusive.
       If not specified its value is 0, 0, 0.

CategoryInfo Format
^^^^^^^^^^^^^^^^^^^
This subsection of an AttDef Element describes the local category constants imposed on the Definition.  All Item Definitions belonging to
this Definition, as well as all Definitions derived from this, will inherit these constraints unless explicitly told not to.

The category information is divided into two elements:

* <Include> - represents the set of categories that will make this Definition relevant
* <Exclude> - represents the set of categories that will make this Definition not relevant.

The children of these elements include <Cat> elements whose values are category names.  These elements also include an XML attribute called **Combination** which can be set to *Any* or *All*.
If set to *All*, all of the categories listed must be active in the owning attribute resource in order for the set to be considered *satisfied*.  If *Any* is specified, then only one of the categories need to be active in order for the set to be *satisfied*.

The CategotyInfo Element itself can have the following XML Attributes:

.. list-table:: XML Attributes for <CategoryInfo> Element
   :widths: 10 40
   :header-rows: 1
   :class: smtk-xml-att-table

   * - XML Attribute
     - Description

   * - Combination
     - String value representing how the Inclusion and Exclusion sets should be combined.

       If set to *All*, then both the Inclusion set must be satisfied and the Exclusion set must not be *satisfied* in order for the Definition (and the Attributes it generates) to be relevant.  If it is set to *Any*, then either the Inclusion set is satisfied or the Exclusion set is not, in order for the Definition (and the Attributes it generates) to be relevant.

   * - Inherit
     - Boolean value that indicates if the Definition should inherit its Base Definition's categories.


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
     - Defines the categories that the item belongs to.
       (Optional)

       This element contains at set of <Cat> elements with each
       containing a category defined is the Category Section.

       See `Category Section`_.

   * - <BriefDescription>
     - Provides a brief description of the item (Optional).

   * - <DetailedDescription>
     - Provides a detailed description of the item (Optional).

   * - <DefaultValue>
     - For Integer, String, and Double items, this element's text
       contains the default value for the item. This element is
       not allowed for other ItemDefinition types. (Optional)

       For items that are not discrete and not extensible but do have
       NumberOfRequiredValues greater than 1, it is possible to
       provide a different default value for each component.
       In this case, commas are assumed to separate the values.
       If you wish to use a different separator, specify the "Sep"
       attribute on the DefaultValue tag.

       For example, a String item with 3 components might use

       .. code:: xml

          <DefaultValue Sep=":">Oh, my!:Oh no!:Yes, please.</DefaultValue>

       to specify different defaults for each component.
       You can also use the separator to prevent a default value
       from having per-component values. For instance, the same
       String item might use

       .. code:: xml

          <DefaultValue Sep=":">Commas, they are my style.</DefaultValue>

       to force the default value to have a single entry used to
       initialize all components.


All of the elements support the following common XML Attributes.  Note
that each element may have additional XML Attributes that are specific to
it.

.. list-table:: Common XML Attributes for Item Definition Elements
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Label
     - String value representing the name of the item being defined.
       (Required)

       Note that this value should be unique with respects to all
       other items contained within this attribute definition
       (including its Base Type).

   * - Version
     - Integer value representing the "version" of the item.
       (Optional)

       This is used for versioning the item.  If not specified
       then 0 is assumed.

   * - Optional
     - Boolean value indicating if the item is considered optional
       or required.
       (Optional)

       If not specified the item is considered to be required.

   * - IsEnabledByDefault
     - Boolean value indicating if the item is considered to be
       enabled by default.
       (Optional)

       Note this is only used when Optional="true".
       If not specified, the item is considered to be disabled.

   * - AdvanceLevel
     - Integer value used to indicate the advance level associated
       with the item.
       (Optional)

       This value should match one of the advance values
       defined in the `Advance Level Section`_.
       If not specified, 0 is assumed.

   * - AdvanceReadLevel
     - Integer value used to indicate the advance read level associated
       with the item.
       (Optional)

       This value should match one of the advance values
       defined in the `Advance Level Section`_.
       Note that this is ignored if the AdvanceLevel XML Attribute is used.

       If not specified, 0 is assumed.

   * - AdvanceWriteLevel
     - Integer value used to indicate the advance write level associated
       with the item.
       (Optional)

       This value should match one of the advance values
       defined in the `Advance Level Section`_.
       Note that this is ignored if the AdvanceLevel XML Attribute is used.

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

Attribute Properties
""""""""""""""""""""

The Attribute node can contain a **Properties** node that has the same format as the Properties node for the Resource.

File Item  <File>
"""""""""""""""""""""""""""
.. todo::

   Describe file items and how they are serialized

Group Item  <Group>
"""""""""""""""""""""""""""""
.. todo::

   Describe group items and how they are serialized

Integer Item  <Int>
"""""""""""""""""""""""""""""
.. todo::

   Describe integer items and how they are serialized

String Item  <String>
"""""""""""""""""""""""""""""""
.. todo::

   Describe string items and how they are serialized

Ref Item  <Ref>
"""""""""""""""""""""""""
.. todo::

   Describe attribute reference items and how they are serialized

Model Entity Item  <Model>
""""""""""""""""""""""""""""""""""""""""""

A :smtk:`ModelEntityItem`, which appears in XML as a <Model> is an
item belonging to an attribute stored as a UUID that refers to an
SMTK model entity.
These model entities may be regions, faces, edges, vertices, or even
higher-level conceptual entities such as models, groups, or instances (used
in modeling scene graphs and assemblies).

Void Item Definition <Void>
"""""""""""""""""""""""""""
.. todo::

   Describe "void" items and how they are serialized

Views Section <Views>
---------------------

The Views section of an SBT file contains multiple <View> items,
one of which should be marked as a top-level view by adding the
TopLevel attribute to it. The top-level view is typically composed
of multiple other views, each of which may be defined in the
Views section.

Each <View> XML element has attributes and child elements that
configure the view.
The child elements may include information about which attributes
or classes of attributes to present as well as how individual items
inside those attributes should be presented to the user.

.. list-table:: Common XML Attributes for View Definition Elements
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Type
     - An enumeration that specifies what information the view should
       present and dictates the children XML elements it must or may contain.
       Acceptable values include: "Group" (a view that groups child views
       in tabs), "Instanced" (a view that displays attribute instances
       specified by their names), "Attribute" (a view that displays all
       attributes sharing a common definition and usually allows users to
       manage instances by adding and removing them), "Operator" (a view
       that customizes how operation parameters are displayed), "ModelEntity"
       (a view that shows model entities and lets users choose an attribute
       to associate with each one, optionally creating instances as needed),
       "SimpleExpression", "Category", or "Selector".
       (Required)

   * - Title
     - A string that summarizes the view to a user.
       When a view is tabbed inside another, the title string
       serves as the label for the tab.
       (Required)

   * - TopLevel
     - Boolean value indicating whether the view is the root view
       that should be presented to the user or (if TopLevel is
       false or omitted) the view is a child that may be included
       by the toplevel view.


.. todo::

   Describe root views and how they are serialized

View configuration
^^^^^^^^^^^^^^^^^^

Each view Type above may need configuration information
specified by child XML elements of the <View>.
The sections below define those child element types:
the first section covers how a view chooses what attributes
to show while the second section covers ways to customize
how those attributes' items are presented.

Attribute selection
~~~~~~~~~~~~~~~~~~~

.. todo::

   Describe <InstancedAttributes>, <Attributes>, ... elements here.

Item Views
~~~~~~~~~~

If you wish to customize how items of the chosen attributes are presented,
you should add an <ItemViews> child to the <Att> tags in the attribute selectors
covered in the previous section.
Inside <ItemViews> you can add a <View> tag for each item whose appearance you
wish to customize.

.. todo::

  Describe <ItemsViews> children

.. list-table:: XML Attributes for Item View Elements
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Type
     - The name of a widget you would like the application to use to present the
       item (and potentially its children as well) to users.
       SMTK provides "Box" (for showing a 3-D bounding box widget), "Point" (for
       showing a 3-D handle widget), "Line", "Plane", "Sphere", "Cylinder", "Spline".
       These types usually only make sense for Group items, although "Box" can also
       be used for a Double item with 6 required values (xmin, xmax, ymin, ymax, zmin, zmax).
       Items may also have custom views provided by the application or external libraries;
       in that case, the Type specifier will be the string used to register the custom view.

   * - ItemMemberIcon, ItemNonMemberIcon
     - These attributes are only used for ReferenceItem, ResourceItem, and ComponentItem
       classes. They specify paths (either on disk or, with a leading colon, to Qt resource files)
       to images to use as icons that display whether a persistent object is a member
       of the item or not.
