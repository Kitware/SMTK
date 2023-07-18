Template File Syntax (Reference) for Version 7 XML Attribute Files
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

       Current value is 7 (latest version)

   * - DisplayHint
     - Boolean value that indicates if the Attribute Resource should be automatically
       displayed in an Attribute Editor when loaded into memory.

       (Optional – default is false which means the Resource will not automatically be
       displayed)

   * - TemplateType
     - A string providing a name for the workflow this attribute describes.

       (Optional – the default is an empty string.)

   * - TemplateVersion
     - An integer describing the version number of the schema for this attribute
       resource. The default value is 0.

This element can contain the following optional children XML Elements:

- Includes : used for including additional attribute files (see `Includes Section`_)
- AdvanceLevels : used to define various advance access levels used in
  GUIs (see `Advance Level Section`_)
- Categories : used to define workflow specific categories (see `Category Section`_)
- Analyses : used to define various analysis groups (see `Analysis Section`_)
- Analysis Configurations: predefined analysis configuration from which the user can choose from (see `Analysis Configuration Section`_)
- ItemBlocks : used to define reusable  blocks of Attribute/Item Definitions Information (see `Item Blocks Section`_)
- Templates : used to define reusable  parameterized blocks of Attribute/Item Definitions Information(see `Templates Section`_)
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
not be repeated among sibling Level Elements.  The higher the value
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
       in the GUI (Optional - else the label will be the word *Level* followed by the advance level)

   * - Color
     - String value representing the color to be used when displaying
       items that are associated with this access level.  The format
       is "r, g, b" where r, g, and b are a value between 0 and 1
       inclusive (Optional)


Category Section
----------------
This is an optional section describing the set of categories used
within the file.  Attribute and Items Definitions can use these categories to define their category constraints.  In addition, analyses are defined as sets of
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
If you include an Attribute XML file that also assigns Resource Properties, the include file's Properties are assigned first.  Meaning that the file using the include file can override the Properties set by the include file.

**Note** - the ability to unset a Property is currently not supported.

**Note** - Properties are currently not saved in the XML format, but are saved if using the JSON format.

Analysis Section
----------------
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

An analysis can be composed of sub-analyses from which the user can choose.  By default, the user can select a subset of these sub-analyses as shown below:

  .. findfigure:: analysisExample1.*
   :align: center
   :width: 90%

Here you see analysis **B**  is composed of 2 optional sub-analyses **B-D** and **B-E**.  In this case **B-E** has been selected.  An active sub-analysis will add its categories to those of its parent's analysis.  The above example could be rewritten using the concept of sub-analyses:

.. code-block:: xml

  <Analyses>
    <Analysis Type="Groundwater Flow">
      <Cat>Flow</Cat>
      <Cat>General</Cat>
      <Cat>Time</Cat>
    </Analysis>
    <Analysis Type="Groundwater Flow with Heat Transfer" BaseType="Groundwater Flow">
      <Cat>Heat Transfer</Cat>
    </Analysis>
  </Analyses>

You can also indicate if the user must select only one sub-analysis by indicating that the base analysis is **Exclusive**.  In the example below, analysis **C** has been marked **Exclusive** and consists of two sub-analyses **C-D** and **C-E**.  The user must select one of these analyses.

  .. findfigure:: analysisExample2.*
   :align: center
   :width: 90%

If you wish to have the user exclusively choose among the top level analyses, you can add the Exclusive attribute to the Analyses XML node itself as shown below.

.. code-block:: xml

  <Analyses Exclusive="true">
    <Analysis Type="A">
      <Cat>A</Cat>
    </Analysis>
    <Analysis Type="B">
      <Cat>B</Cat>
    </Analysis>
  </Analyses>

Please see smtk/data/attribute/attribute_collection/SimpleAnalysisTest.sbt  smtk/data/attribute/attribute_collection/analysisConfigurationExample.sbt for complete examples which can be loaded into ModelBuilder.

Analysis Element Format
^^^^^^^^^^^^^^^^^^^^^^^
Each Analysis is defined within an <Analysis> XML Tag.

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

   * - BaseType
     - String value representing the type of the analysis this analysis is a sub-analysis of.
       (Optional)

   * - Exclusive
     - Boolean value indicating the sub-analyses of this analysis are exclusive.
       (Optional)

   * - Required
     - Boolean value indicating the analysis is required and not optional.
       (Optional)
       **Note** - Required Analyses can not be sub-analyses of an Analyses with **Exclusive** set to *true*.


Each Analysis element contains a set of Cat XML Elements.

Analysis Configuration Section
------------------------------
This is an optional section that define analyses configurations.  The `Analysis Section`_ describes relationship between analyses, while the configuration section defines various analysis configurations from which the user can choose from.  Here is an example configuration section from smtk/data/attribute/attribute_collection/analysisConfigurationExample.sbt:

.. code-block:: xml

  <Configurations AnalysisAttributeType="Analysis">
    <Config Name="Test A" AdvanceReadLevel="5">
      <Analysis Type="A"/>
    </Config>
    <Config Name="Test B" AdvanceWriteLevel="10">
      <Analysis Type="B"/>
    </Config>
    <Config Name="Test B-D">
      <Analysis Type="B">
        <Analysis Type="B-D"/>
      </Analysis>
    </Config>
    <Config Name="Test C">
      <Analysis Type="C"/>
    </Config>
    <Config Name="Test C-D">
      <Analysis Type="C">
        <Analysis Type="C-D"/>
      </Analysis>
    </Config>
    <Config Name="Test C-E">
      <Analysis Type="C">
        <Analysis Type="C-E"/>
      </Analysis>
    </Config>
    <Config Name="Test C-E-F">
      <Analysis Type="C">
        <Analysis Type="C-E">
          <Analysis Type="C-E-F"/>
        </Analysis>
      </Analysis>
    </Config>
  </Configurations>


.. list-table:: XML Attributes for <Configurations> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - AnalysisAttributeType
     - String value representing the type name for the Attribute Definition used to represent an Analysis Configuration.
       (Required)

The Configuration Element is composed of a set of Config Elements.  Each Config Element represents an Analysis Configuration.

.. list-table:: XML Attributes for <Config> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Name
     - String value representing the configuration's name (which will also be the name of an Analysis Attribute that represents that
       configuration). (Required)

   * - AdvanceReadLevel
     - Integer value representing the configuration's read access level.  If the advance level is set below the configuration's
       AdvanceReadLevel, the configuration will not be displayed to the user. (Optional - default is 0)

   * - AdvanceWriteLevel
     - Integer value representing the configuration's write access level.  If the advance level is set below the configuration's
       AdvanceWriteLevel, the user will not be allowed to edit the configuration.
       (Optional - default is 0)

Each Config Element consists of a nested group of Analysis Elements.  Each Analysis Element represents an analysis defined in the `Analysis Section`_.  Specifying an Analysis indicates that the analysis is active when the configuration is selected.  If the analysis is **Exclusive** then it **must** contain an Analysis Element referring to one of its sub-analyses.  If it is **not Exclusive** then it can optionally contain multiple Analysis Elements indicating which sub-analyses should be active.

.. list-table:: XML Attributes for <Analysis> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Type
     - String value representing an analysis type.

Unique Roles Section
--------------------
There are use cases where the developer would like to enforce a constraint among ComponentItems such that each item cannot point to the same resource component. In order to provide this functionality, we have introduced the concept of unique roles.  Roles in this context refers to the roles defined in the resource links architecture and that are referenced in ReferenceItemDefinition.  If the role specified in either a Reference or Component Item Definition is include in this section, it is considered **unique**.

Here is an example of an Unique Role Section

.. code-block:: xml

  <UniqueRoles>
    <Role ID="10"/>
    <Role ID="0"/>
  </UniqueRoles>

In this case roles 10 and 0 are considered to be unique.

Item Blocks Section
---------------------------------
Item Definition Blocks allows the reuse of a group of Attribute / Item Definitions Components in different Attribute/Item Definitions.  Providing a "hasA" relationship as opposed to the currently supported "isA". These blocks can then be referenced in the "ItemDefinitions" nodes of Attribute or Group Item Definitions or in the "ChildrenDefinitions" nodes for Reference or Value Item Definitions. Blocks themselves can reference other blocks.  But care must be taken not to form a recursive relationship.  In the parser detects such a pattern it will report an error.

When referencing a Block, the items will be inserted relative to where the Block is being referenced.

Category constraints are inherited as usual and that Blocks can call other blocks.  Here is an example of an Item Block:

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
    <AttDef Type="Type3">
      <Categories>
        <Cat>Heat Transfer</Cat>
      </Categories>
      <ItemDefinitions>
        <Block Name="B1" Namespace="globals2"/> <!-- An example of referring to an Item Block that is defined in namespace *globals2* -->
        <String Name="str2"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

See data/attribute/attribute_collection/ItemBlockTest.sbt and smtk/attribute/testing/cxx/unitItemBlocks.cxx for examples.

The Item Block functionality has been extended so that an Item Block can be exported in one file and consumed in another using the **Export** XML attribute.  In order to
better organize  Item Blocks, we have also introduced the concept of a *namespace* that behaves similar to namespaces in C++.  A namespace scopes an
Item Block and therefore must also be used when referring to an Item Block.

In Version 7 XML files, Item Blocks functionality has been further extended. Blocks can be used within XML Elements:

 * Definitions
 * AttDef
 * Any ValueItemDefinition

 For example, you can use a Block to contain the discrete information of a String ItemDefinition.

**Note** Namespaces are only used w/r Item Blocks (and now Templates) and they can not be nested.

.. list-table:: XML Attributes for <ItemBlocks> Element
   :widths: 10 40
   :header-rows: 1
   :class: smtk-xml-att-table

   * - XML Attribute
     - Description

   * - Namespace
     - String value representing the default namespace to which all of its Item Blocks belong to. (Optional)

       **Note** that if not specified SMTK assumes that the default namespace is the global namespace represented by ""

.. list-table:: XML Attributes for <ItemBlock> Element
   :widths: 10 40
   :header-rows: 1
   :class: smtk-xml-att-table

   * - XML Attribute
     - Description

   * - Name
     - String value representing the name of the ItemBlock.
       (Required)

   * - Namespace
     - String value representing the  namespace that the Item Block belongs to.
       (Optional)

       **Note** If not specified it will use the default namespace specified in the **ItemBlocks** node.

   * - Export
     - Boolean indicating that the ItemBlock should be made available to template files that include this one.
       (Optional)

       **Note** If not specified SMTK assumes that the value is *false* meaning that the ItemBlock is file-scope only

Templates Section
---------------------------------
Templates are a further extension of Item Block's functionality by provided the ability to parameterize the information. A Template Definition is composed of two elements.  **Contents** represents the information to be inserted.  This is similar to the contents of an ItemBlock.  It also can contain an optional **Parameters** element that defines the parameters used in the Template.  When referring to a template parameter, you need to enclose it in {}.  A declared parameter may be given a default value.  When instantiating a Template, you can include a set of Parameters along with their values.  **You must include values for all parameters that were not given a default value in the Template's Definition.** A Template can be used anywhere an ItemBlock can be used.

When instantiating a Template, the infromation will be inserted relative to where the Template is being referenced.

Category constraints are inherited as usual and that Templates can call other Templates.  Here is an example of using a Template:

.. code-block:: xml

  <Templates>

    <Template Name="SimpleStringDefault">
      <Parameters>
        <Param Name="a">dog</Param>
      </Parameters>
      <Contents>
        <DefaultValue>{a}</DefaultValue>
      </Contents>
    </Template>

    <Template Name="SimpleAttribute">
      <Parameters>
        <Param Name="type"/>
      </Parameters>
      <Contents>
        <AttDef Type="{type}">
          <ItemDefinitions>
            <String Name="s1">
              <Template Name="SimpleStringDefault">
                <Param Name="a">cat</Param>
              </Template>
            </String>
            <String Name="s2">
              <Template Name="SimpleStringDefault"/>
            </String>
            <String Name="s3">
              <Template Name="DiscreteStringInfo">
                <Param Name="defaultIndex">1</Param>
              </Template>
            </String>
          </ItemDefinitions>
        </AttDef>
      </Contents>
    </Template>
  </Templates>

  <Definitions>
    <Template Name="SimpleAttribute">
      <Param Name="type">A</Param>
    </Template>
  </Definitions>

See data/attribute/attribute_collection/TemplateTest.sbt and smtk/attribute/testing/cxx/unitTemplates.cxx for examples.

Templates are supported in Version 7 (and later) XML files.

**Note** As mentioned previously, Namespaces are only used w/r Templates and ItemBlocks and they can not be nested.

.. list-table:: XML Attributes for <Templates> Element
   :widths: 10 40
   :header-rows: 1
   :class: smtk-xml-att-table

   * - XML Attribute
     - Description

   * - Namespace
     - String value representing the default namespace to which all of its Templates belong to. (Optional)

       **Note** that if not specified SMTK assumes that the default namespace is the global namespace represented by ""

.. list-table:: XML Attributes for <Template> Element
   :widths: 10 40
   :header-rows: 1
   :class: smtk-xml-att-table

   * - XML Attribute
     - Description

   * - Name
     - String value representing the name of the Template.
       (Required)

   * - Namespace
     - String value representing the  namespace that the Template belongs to.
       (Optional)

       **Note** If not specified it will use the default namespace specified in the **Templates** node.

   * - Export
     - Boolean indicating that the Template should be made available to XML attribute files that include this one.
       (Optional)

       **Note** If not specified SMTK assumes that the value is *false* meaning that the Template is file-scope only


.. list-table:: XML Children Elements for <Template> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description

   * - <Parameters>
     - Defines the parameters used by the Template
       (Optional).

   * - <Contents>
     - Defines the information to be inserted when the template is instanced.


.. list-table:: XML Children Elements for <Parameters> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description

   * - <Param>
     - Defines a parameter used by the Template.  If the contents of the element is not empty, it will be assumed to be the default value of the parameter

.. list-table:: XML Attributes for <Param> Element
   :widths: 10 40
   :header-rows: 1
   :class: smtk-xml-att-table

   * - XML Attribute
     - Description

   * - Name
     - String value representing the name of the Parameter.
       (Required)

   * - OkToBeEmpty
     - Boolean indicating that the parameter has an empty default.
       (Optional)


Definitions Section
---------------------------------
This is an optional section that defines a set of attribute
definitions used to generate attributes with a SMTK-based program.
This section is created using the <Definitions> XML Element.
See :ref:`the example XML <XMLExample>` for how to create a set
of attribute definitions.

This element can contain the following children XML Elements:


.. list-table:: XML Children Elements for <Definitions> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description

   * - <AttDef>
     - Defines a new Attribute Definition

       See `AttDef Element Format`_.

   * - <Exclusions>
     - Defines a set of Definitions whose Attributes would mutually exclude each other from being associated
       to the same Resource/Resource Component
       (Optional).

       See `Exclusions Format`_.

   * - <Prerequisites>
     - Defines a set of Definitions whose Attributes are required to be
       associated to a Resource or Resource Component prior to being able to
       associated  Attributes from another Definition to the same Resource or Resource Component
       (Optional).
       See `Prerequisites Format`_.



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

       See `CategoryInfo Format`_.

   * - <ItemDefinitions>
     - Defines the items contained within the attributes generated
       by this definition (Optional).

       See `Item Definitions Format`_.

   * - <AssociationsDef>
     - Defines the association rules associated with this Definition (Optional).
       See `AssociationsDef Format`_.

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
       with the definition and the attributes it generates for reading and modifying.
       (Optional)

       This value should match one of the advance values
       defined in the `Advance Level Section`_.
       If not specified, 0 is assumed.

   * - AdvanceReadLevel
     - Integer value used to indicate the advance level associated
       with the definition and the attributes it generates for reading.
       (Optional)

       This value should match one of the advance values
       defined in the `Advance Level Section`_.
       If not specified, 0 is assumed.

       **Note** if the *AdvanceLevel* is also specified, *AdvanceReadLevel* will be ignored.

   * - AdvanceWriteLevel
     - Integer value used to indicate the advance level associated
       with the definition and the attributes it generates for modifying.
       (Optional)

       This value should match one of the advance values
       defined in the `Advance Level Section`_.
       If not specified, 0 is assumed.

       **Note** if the *AdvanceLevel* is also specified, *AdvanceWriteLevel* will be ignored.

   * - Unique
     - Boolean value used to indicate if the attributes this definition
       generates are unique with respects to the model entities it
       associated with.
       A resource or resource component can only have one unique attribute of a given
       type associated with it.
       (Optional)

       If not specified, the definition is assumed to be non-unique.

   * - Nodal
     - Boolean value used to indicate if the attribute effects the
       nodes of the analysis mesh or the elements.
       (Optional)

       If not specified the definition's attributes are not nodal.

   * - RootName
     - String value used to auto-generate names for attributes.
       (Optional)

       If not specified the definition's type name is used.

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

       **Note** This is an old format and is only for model resource components.
       Instead you should be using an **AssociationsDef** XML Element.

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
~~~~~~~~~~~~~~~~~~~
This subsection of an AttDef Element describes the local category constants imposed on the Definition.  All Item Definitions belonging to
this Definition, as well as all Definitions derived from this, will inherit these constraints unless explicitly told not to.

The category information is divided into two elements:

* <Include> - represents the set of categories that will make this Definition relevant
* <Exclude> - represents the set of categories that will make this Definition not relevant.

The children of these elements include <Cat> elements whose values are category names.  These elements can optionally include an XML attribute called **Combination** which can be set to *Or* or *And*.
If set to *And*, all of the categories listed must be active in the owning attribute resource in order for the set to be considered *satisfied*.  If *Or* is specified, then only one of the categories need to be active in order for the set to be *satisfied*.  The default is *Or*.

The CategotyInfo Element itself can have the following XML Attributes:

.. list-table:: XML Attributes for <CategoryInfo> Element
   :widths: 10 40
   :header-rows: 1
   :class: smtk-xml-att-table

   * - XML Attribute
     - Description

   * - Combination
     - String value representing how the Inclusion and Exclusion sets should be combined.

       If set to *And*, then both the Inclusion set must be satisfied and the Exclusion set must not be *satisfied* in order for the Definition (and the Attributes it generates) to be relevant.  If it is set to *Or*, then either the Inclusion set is satisfied or the Exclusion set is not, in order for the Definition (and the Attributes it generates) to be relevant.

       (Optional - Default is *And*)

   * - InheritanceMode
     - String value that indicates how the Definition should combine its local category information with the category information
       inherited from its Base Definition.

       If set to *And*, then its local category information will be and'd with its Base Definition's.  If it is set to *Or*, then its local category information will be or'd with its Base Definition's.  If it is set to *LocalOnly*, then the Base Definition's category information will be ignored.

       (Optional - the default is *And*)


AssociationsDef Format
~~~~~~~~~~~~~~~~~~~~~~
The format is an extension of the `Reference Item Based Definitions`_. When dealing with Model Resource Components you can use a **MembershipMask** XML Element to indicate the type of Model Resource Component that can be associated.

.. code-block:: xml

      <AssociationsDef Name="BC Sets" NumberOfRequiredValues="0" Extensible="true">
        <MembershipMask>edge</MembershipMask>
      </AssociationsDef>

The following table shows the XML
Attributes that can be included in this XML Element in addition to those supported by Reference Item Definition.

.. list-table:: XML Attributes for <AssociationsDef> Element
   :widths: 10 40
   :header-rows: 1
   :class: smtk-xml-att-table

   * - XML Attribute
     - Description

   * - OnlyResources
     - Boolean value indicating that the association is only to Resources. (Optional).

   * - <ReferenceLabels>
     - Defines the labels that should be displayed
       next to the Item's values.
       This element should only be specified if the Item is either **Extensible** or has **NumberOfRequiredValues** > 1.

       See `Specifying Labels`_.
       (Optional)

Exclusions Format
^^^^^^^^^^^^^^^^^
Contains a set of Definition exclusion rules.  An exclusion rule is a set of Definitions that exclude each other. This means an Attribute from one of these excluded Definition that is associated with a Resource/ Resource Component will prevent attributes from any of the other excluded Definitions from being associated to the same Resource/Resource Component.

.. list-table:: XML Children Elements for <Exclusions> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description

   * - <Rule>
     - Defines a new Exclusion Rule

Each exclusion rule is a set of Def XML Elements.  Each Def XML Element contains the name of a Definition type.

**Note** A rule must contain at least 2 Def Elements.

.. list-table:: XML Children Elements for <Rule> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description

   * - <Def>
     - Contains a Definition Type

Here is an example of an Exclusion Rule where Definitions A and B exclude each other:

.. code-block:: xml

 <Exclusions>
    <Rule>
      <Def>A</Def>
      <Def>B</Def>
    </Rule>
  </Exclusions>

Prerequisites Format
^^^^^^^^^^^^^^^^^^^^
Contains a set of Definition prerequisite rules.  An prerequisite rule refers to a Definition and a set of prerequisite Definitions.

.. list-table:: XML Children Elements for <Prerequisites> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description

   * - <Rule>
     - Defines a new Prerequisite Rule

Each Prerequisite rule refers to a Definition that has the prerequisite which is represented  as a set of Definitions.

.. list-table:: XML Attributes for <Rule> Element
   :widths: 10 40
   :header-rows: 1
   :class: smtk-xml-att-table

   * - XML Attribute
     - Description

   * - Type
     - String value representing the Definition type that has the prerequisite

.. list-table:: XML Children Elements for <Rule> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description

   * - <Def>
     - Contains a Definition Type

Here is an example of an Exclusion Rule where Definition A has a prerequisite requiring that an attribute of Type C must be associated to an object in order to be able to associate an attribute of type A to the same object.

.. code-block:: xml

 <Prerequisites>
    <Rule Type="A">
      <Def>C</Def>
    </Rule>
  </Prerequisites>


Item Definitions Format
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
This subsection of an AttDef Element contains the definitions of all the
items to be created within the attributes created by the attribute
definition.  The section is represented by the <ItemDefinitions> XML
Element and can contain any of the elements described in the Item
Definition Section.

Item Definition Section
------------------------
All of the XML Elements described within this section can be added to
any of the following elements:

* <ItemDefinitions> of an attribute definition <AttDef> or Group Item Definition <Group>
* <ChildrenDefinitions> of a Value Item Definition (<Double>, <Int>, <String>)

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

This element can contain the following children XML Elements:

.. list-table:: Common XML Children Elements for Item Definition Elements
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description

   * - <CategoryInfo>
     - Specifies the local category constraints specified on the
       Item Definition.  The format is identical to the one used for Attribute Definitions except
       that the category inheritance mode controls how the Item Definition's local category information is
       combined with the category information it inherits from either its owning Attribute or Item Definition.
       (Optional)

       See `CategoryInfo Format`_.

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

   * - Name
     - String value representing the name of the item being defined.
       (Required)

       Note that this value should be unique with respects to all
       other items contained within this attribute definition
       (including its Base Type).

   * - Label
     - String value representing the label that should be displayed in the GUI
       for the item.
       (Optional - if not specified the Item's Name will be used)

       Note if it is set to ' ', this will indicate that no label should be displayed..

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

Simple Void Items
^^^^^^^^^^^^^^^^^

A Void Item Definition, represented  by a <Void> XML Element, is used to represent boolean information.  By itself, it contains no additional information beyond what is
specified for the information above and its corresponding item *value* rely solely on its *enabled* state.  As a result, Void Items are almost always *Optional*.

Basic Value Items
^^^^^^^^^^^^^^^^^^
Basic Value Items include Strings, Integers and Doubles.  They are represented as follows:


.. list-table:: XML Elements for Basic Value Item Definition.
   :widths: 10 40
   :header-rows: 1

   * - XML Element
     - Description

   * - <Int>
     - Represents an Integer Item Definition

   * - <Double>
     - Represents a Double Item Definition

   * - <String>
     - Represents a String Item Definition


.. list-table:: Common XML Attributes for Value Item Definition Elements
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - NumberOfRequiredValues
     - Integer value representing the minimum number of values the Item should have.
       (Optional - if not specified assumed to be 1)

   * - Extensible
     - Boolean value indicating that the Item's number of values can be extended past **NumberOfRequiredValues**.
       (Optional - if not specified assumed to be *false*)

   * - MaxNumberOfValues
     - Integer value representing the maximum number of values the Item can have.  A value of 0 means there is no
       maximum limit.
       (Optional - if not specified assumed to be 0)

       **Note** - this is only used if **Extensible** is *true*.

   * - Units
     - String value representing the units the Item's value(s) are specified in.
       (Optional - if not specified assumed to be "")



.. list-table:: Common XML Children Elements for Basic Value Item Definition Elements
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description

   * - <DefaultValue>
     - For Integer, String, and Double items, this element's text
       contains the default value for the item. This element is
       not allowed for other ItemDefinition types.
       See `Specifying Default Values`_.
       (Optional)

   * - <RangeInfo>
     -  Defines a value range for the Item's values.

        See `Specifying Ranges`_.
        (Optional - if not specified there is no range constraint.)

   * - <ComponentLabels>
     - Defines the labels that should be displayed
       next to the Item's values.
       This element should only be specified if the Item is either **Extensible** or has **NumberOfRequiredValues** > 1.

       See `Specifying Labels`_.
       (Optional)

   * - <ExpressionType>
     - Indicates that the Item's value(s) can be represented using an expression of the indicated type.
       (Optional)

   * - <DiscreteInfo>
     - Indicates that the Item is restricted to a discrete set of values
       See `Modeling Discrete Information Section`_
       (Optional)

   * - <ChildrenDefinitions>
     - A set of Item Definitions that are referenced in the Item's <DiscreteInfo> element.
       The contents is identical to an Attribute Definition's <ItemDefinitions> element.
       (Optional)

Specifying Default Values
~~~~~~~~~~~~~~~~~~~~~~~~~

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

Specifying Ranges
~~~~~~~~~~~~~~~~~
The RangeInfo Element can indicate the min and/or max value for the an Item's value(s) as well as indicating if these
values are inclusive or exclusive.  This element is used for Double or Integer Item Definitions.

.. list-table:: XML Children Elements for <RangeInfo> Elements
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description

   * - <Min>
     - Sets the minimum value for the Item's value(s)
       (Optional)

   * - <Max>
     - Sets the maximum value for the Item's value(s)
       (Optional)

.. list-table:: XML Attributes for <Min> / <Max> Elements
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Inclusive
     - Boolean value indicating if an Item's value can be set to the min / max value.
       (Optional - if not specified it is assumed to be *false*)

In the example below, CommonDouble must be strictly > 0.

.. code-block:: xml

        <Double Name="CommonDouble" Label="Floating Pt Val" Version="0" NumberOfRequiredValues="1">
          <DefaultValue>3.1415899999999999</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0</Min>
          </RangeInfo>
        </Double>


Specifying Labels
~~~~~~~~~~~~~~~~~

.. list-table:: XML Attributes for <ComponentLabels> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - CommonLabel
     - String Value representing the label that should precede each of the Item's values when being displayed
       (Optional)

If each value requires a different label, this element would contain a set of <Label> elements (one for each value) as shown below:

.. code-block:: xml

  <ComponentLabels>
    <Label>t</Label>
    <Label>u</Label>
    <Label>v</Label>
    <Label>w</Label>
  </ComponentLabels>

**Note** - Individual labels are not supported for extensible Items.


Modeling Discrete Information Section
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Used when a Double, Integer, or Double Item is restricted to a set of values.  Each value can optionally have an enumeration string associated with that would be displayed
in an UI instead of the value in order to improve readability.  In addition, an enumeration can have categories associated with it, indicating that
in addition to satisfying the Item's category requirements, these requirements must also be met in order for the Item to be set to that particular value.
An enumeration can also have an Advance Level associated with it.

An enumeration can also refer to a subset of the Item's Children Definitions indicating additional Items that need to be filled out when set to that value.


.. list-table:: XML Attributes for <DiscreteInfo> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - DefaultIndex
     - Integer value representing the index of the the Item's discrete value to be used as the Item's default value.

.. list-table:: XML Children Elements for <DiscreteInfo> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description

   * - <Value>
     - Simple form when defining only discrete values without category information or associated Item Definitions.

   * - <Structure>
     - Complete form which contains a <Value> Element

**Note** - The DiscreteInfo element can be composed of a mixture of both types of children elements.

Simple Discrete Value Form
""""""""""""""""""""""""""

The Value Element defines the discrete value as well as an optional enumeration string and/or advance level.

.. list-table:: XML Attributes for <Value> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Enum
     - String value representing the enumeration associated with this discrete value
       (Optional - if not specified, the value will be used)

   * - AdvanceLevel
     - Integer value representing the advance level associated with the value.
       (Optional - if not specified, 0 is assumed)



.. code-block:: xml

  <Value Enum="e3" AdvanceLevel="1">c</Value>


Structured Discrete Value Form
""""""""""""""""""""""""""""""
The Structure Element contains a Value element along with optional category and required Item Definitions.

.. list-table:: XML Children Elements for <Structure> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description

   * - <Value>
     - Simple form when defining only discrete values without category information or associated Item Definitions.
       See `Simple Discrete Value Form`_.

   * - <CategoryInfo>
     - Specifies the additional category constraints associated with this discrete value.
       The format is identical to the one used for Attribute Definitions except
       that the category inheritance mode is not used.
       (Optional)

       See `CategoryInfo Format`_.

   * - <Items>
     - Represents the Children Items that are associated with this discrete value.
       These names should be defined in the Item's **ChildrenDefinitions** element.


.. code-block:: xml

   <Structure>
      <Value Enum="Specific Enthalpy Function">specific-enthalpy</Value>
      <Items>
        <Item>specific-enthalpy</Item>
      </Items>
    </Structure>


Group Item Definitions
^^^^^^^^^^^^^^^^^^^^^^

A Group Item Definition is defined as a vector of Item Definitions.  The Group Item the Definition produces will refer to the corresponding vector of Items produced as
a group.  It can also be used to define a set of choices that the user needs to select from.  In this case, all of the internal Item Definitions are assumed to be optional.

.. list-table:: XML Attributes for <Group> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - NumberOfRequiredGroups
     - Integer value representing the minimum number of Item groups the Item should have.
       (Optional - if not specified assumed to be 1)

   * - Extensible
     - Boolean value indicating that the Item's number of groups can be extended past **NumberOfRequiredGroups**.
       (Optional - if not specified assumed to be *false*)

   * - IsConditional
     - Boolean value indicating that the Group Item represents a vector of choices.
       (Optional - if not specified assumed to be *false*)

       **Note** - this is only used if **Extensible** is *true*.

   * - MinNumberOfChoices
     - Integer value representing the minimum number of choices that need to be selected in order for the item to be considered valid.
       (Optional - if not specified assumed to be 0)

       **Note** - this is only used if **IsConditional** is *true*.

   * - MaxNumberOfChoices
     - Integer value representing the maximum number of choices that can be selected in order for the item to be considered valid.  Setting this to 0
       indicates that there is no maximum.
       (Optional - if not specified assumed to be 0)

       **Note** - this is only used if **IsConditional** is *true*.


.. list-table:: XML Children Elements for <Group> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - <ComponentLabels>
     - Defines the labels that should be displayed
       next to the Item's values.
       This element should only be specified if the Item is either **Extensible** or has **NumberOfRequiredGroups** > 1.

       See `Specifying Labels`_.
       (Optional)

   * - <ItemDefinitions>
     - Defines the items contained within the group generated
       by this Item Definition (Optional).

       See `Item Definitions Format`_.

Here is an example of a Group Item Definition that represents a sets of choices.  Please see smtk/data/attribute/attribute_collection/choiceGroupExample.sbt for more examples.

.. code-block:: xml

        <Group Name="opt1" Label="Pick At Least 2"
          IsConditional="true" MinNumberOfChoices="2" MaxNumberOfChoices="0">
          <ItemDefinitions>
            <String Name="opt1"/>
            <Int Name="opt2"/>
            <Double Name="opt3"/>
          </ItemDefinitions>
        </Group>

It produces the following in ModelBuilder:

  .. findfigure:: ChoiceExample.*
   :align: center
   :width: 90%


Reference Item Based Definitions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
A Reference Item Definition creates Items that can refer to other SMTK Persistent Objects such as Resources and Resource Components.  It makes use of the queries functionally provided by the various resources.  Component and Resource Item Definitions are derived from
Reference Item Definition and therefore have many of the same XML attributes and children components in common.


.. list-table:: Common XML Attributes for Reference Item Based Definitions
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - NumberOfRequiredValues
     - Integer value representing the minimum number of values the Item should have.
       (Optional - if not specified assumed to be 1)

   * - Extensible
     - Boolean value indicating that the Item's number of values can be extended past **NumberOfRequiredValues**.
       (Optional - if not specified assumed to be *false*)

   * - MaxNumberOfValues
     - Integer value representing the maximum number of values the Item can have.  A value of 0 means there is no
       maximum limit.
       (Optional - if not specified assumed to be 0)

       **Note** - this is only used if **Extensible** is *true*.

   * - EnforceCategories
     - Boolean value indicating if the reference item's validity check should also
       check to see if the item is pointing to an Attribute that would be considered
       relevant based on the active categories

   * - HoldReference
     - Boolean value that indicates whether the item should be forcing the Persistent Object
       to be kept in memory.
       (Optional: Default is false)

       **Note** - this is currently used for Operation Parameters to make sure
       removed Resources and/or Components are kept in memory after the operation runs.

   * - LockType
     - String value that indicates whether the resource being referred to should
       be locked.  Acceptable values are : DoNotLock, Read or Write
       (Optional)

       **Note** - this is currently used for Operation Parameters only!

   * - Role
     - Integer value that sets the Role of the links used by the Reference Item.
       The main use for setting the Role is to use an Unique Role.  See `Unique Roles Section`_.

.. list-table:: Common XML Children Elements for Reference Item Based Definitions
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - <Accepts>
     - Defines the set of acceptance rules. In order for a SMTK Persistent Object to
       be referenced by this item, it must pass at least one of these rules (if any have been
       specified).  These are represented by a set of **Resource** Elements.

       See `Resource Query Format`_.
       (Optional)

   * - <Rejects>
     - Defines the set of rejection rules. In order for a SMTK Persistent Object to
       be referenced by this item, it must not pass any of these rules (if any have been
       specified).  These are represented by a set of **Resource** Elements.

       See `Resource Query Format`_.
       (Optional)

   * - <ComponentLabels>
     - Defines the labels that should be displayed
       next to the Item's values.
       This element should only be specified if the Item is either **Extensible** or has **NumberOfRequiredGroups** > 1.

       See `Specifying Labels`_.
       (Optional)

   * - <ChildrenDefinitions>
     - Defines the items contained within the reference item generated
       by this Item Definition (Optional).

       See `Item Definitions Format`_.

   * - <ConditionalInfo>
     - Defines which of the children items should be considered active based on
       the type of Persistent Object the Item is referencing.

       See `Specifying Conditional Information for Reference Items`_.
       (Optional)

Resource Query Format
~~~~~~~~~~~~~~~~~~~~~
This is represented by a <Resource> Element and represents a query that can be used
by ReferenceItems and Attribute Associations.

.. list-table:: XML Attributes for <Resource> Element for Resource Queries
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Name
     - String value that represents a Resource Type Name.  For example
       smtk::attribute::Resource.  These names are typically defined in the
       Resource class's *smtkTypeMacro*.

   * - Filter
     - String value representing a query that is valid for Resources referred by
       the above **Name** XML attribute.

       (Optional - if not specified, the Item is referencing Resources not Resource Components)

Component Item Definition
~~~~~~~~~~~~~~~~~~~~~~~~~

Component Items can reference a Resource Component and includes all of the structure for Reference Item Based Definitions.

.. list-table:: XML Children Elements for <Component> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - <ComponentLabels>
     - Defines the labels that should be displayed
       next to the Item's values.
       This element should only be specified if the Item is either **Extensible** or has **NumberOfRequiredValues** > 1.

       See `Specifying Labels`_.
       (Optional)


Resource Item Definition
~~~~~~~~~~~~~~~~~~~~~~~~~

Resource Items can reference a Resource and includes all of the structure for Reference Item Based Definitions.

.. list-table:: XML Children Elements for <Resource> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - <ResourceLabels>
     - Defines the labels that should be displayed
       next to the Item's values.
       This element should only be specified if the Item is either **Extensible** or has **NumberOfRequiredValues** > 1.

       See `Specifying Labels`_.
       (Optional)


Specifying Conditional Information for Reference Items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Provides the ability to activate the Item's children items based on what the Item
is currently pointed to.  In the following example, the Item is pointing to a *LiquidMaterial* the Item will have 2 active children named initialTemp and initialFlow, but if it is pointing to a *SolidMaterial* it will only have 1 active child named initialTemp.

.. code-block:: xml

  <ConditionalInfo>
    <Condition Resource="smtk::attribute::Resource" Component="attribute[type='SolidMaterial']">
      <Items>
        <Item>initialTemp</Item>
      </Items>
    </Condition>
    <Condition Component="attribute[type='LiquidMaterial']">
      <Items>
        <Item>initialTemp</Item>
        <Item>initialFlow</Item>
      </Items>
    </Condition>
  </ConditionalInfo>

.. list-table:: XML Attributes for <Condition> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Resource
     - String value that represents a Resource Type Name.  For example
       smtk::attribute::Resource.  These names are typically defined in the
       Resource class's *smtkTypeMacro*.

       (Optional - if not specified, it assumes that *Component*'s  query string
       is applicable to all Resource Components that passes the Item's Accept/Reject
       conditions.)

   * - Component
     - String value representing a Resource Component query that is valid for Resources
       referred by the above **Resource** XML attribute if specified or for Resources
       that passes the Item's Accept/Reject conditions if it is not specified.

       (Optional - if not specified, the Item is referencing Resources and not Resource Components)

**Note** - that at least one of the **Resource** or **Component** XML Attribute must be
specified.

.. list-table:: XML Children Elements for <Condition> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Child Element
     - Description

   * - <Items>
     - Represents the Children Items that are associated with this discrete value.
       These names should be defined in the Item's **ChildrenDefinitions** element.

Attribute Section <Attributes>
------------------------------
.. todo::

   Describe attributes and how they are serialized


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
     - A string that specifies the View's type and should match the
       View class name or alias that was used to register the View
       implementation with the view::Manager

   * - Title
     - A string that represents the name of the View

   * - Label
     - A string that is used in place of the View's title
       when it is displayed in the UI.

       (Optional - if not specified, the View's title is used.)

   * - TopLevel
     - Boolean value indicating whether the view is the root view
       that should be presented to the user or (if TopLevel is
       false or omitted) the view is a child that may be included
       by the toplevel view.

       (Optional - assumes it is not a top-level view if not specified.)


Analysis View
^^^^^^^^^^^^^

.. list-table:: Alias and Type name for Analysis Views
   :widths: 10 40
   :header-rows: 1

   * - Alias
     - Type Name

   * - Analysis
     - smtk::extension::qtAnalysisView


An Analysis View represents the analysis structure defined in an Attribute Resource.  The View will set an Attribute Resource's *Active Categories* based on the choices the user has made in the View.  The View will generate both an Attribute Definition and Attribute based on the XML attributes if needed.

.. code-block:: xml

   <View Type="Analysis" Title="Configurations" AnalysisAttributeName="analysis" AnalysisAttributeType="analysis">
    </View>

.. list-table::  XML Attributes for Analysis Views
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - AnalysisAttributeName
     - A string that defines the name of the Attribute representing the Analysis.

       *Note* - this name should be unique within the Attribute Resource

   * - AnalysisAttributeType
     - A string that defines the type name of the Attribute Definition representing the Analysis structure.

       *Note* - this type name  should be unique within the Attribute Resource

Here in an example UI of an Analysis View.

  .. findfigure:: analysisViewExample.*
   :align: center
   :width: 90%

Associations View
^^^^^^^^^^^^^^^^^

.. list-table:: Alias and Type name for Associations Views
   :widths: 10 40
   :header-rows: 1

   * - Alias
     - Type Name

   * - Associations
     - smtk::extension::qtAssociationView


An Associations View allows users to change an Attribute's associations but does not allow the user to change the Attribute's values.

.. code-block:: xml

    <View Type="Associations" Title="Materials Associations">
      <AttributeTypes>
        <Att Type="Material">
        </Att>
      </AttributeTypes>
    </View>

The XML Element should have one child element called <AttributeTypes>.  The <AttributeTypes> element is composed of a set of <Att> Elements.  Each <Att> must contain an XML Attribute named **Type** which refers to an Attribute Definition type name.

The View will provide a drop-down list of all smtk::attribute::Attributes that are of (or derived from) one of the specified Attribute Definition types.


Here in an example UI of an Associations View.

  .. findfigure:: associatioViewExample.*
   :align: center
   :width: 90%

Attribute View
^^^^^^^^^^^^^^^^^

.. list-table:: Alias and Type name for Associations Views
   :widths: 10 40
   :header-rows: 1

   * - Alias
     - Type Name

   * - Attribute
     - smtk::extension::qtAttributeViewView


An Attribute View allows users to create, modify, and delete smtk::attribute::Attributes.

.. code-block:: xml

    <View Type="Attribute" Title="Materials">
      <AttributeTypes>
        <Att Type="Material"/>
      </AttributeTypes>
    </View>

.. list-table:: XML Attributes specifically for Attribute View Elements
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - DisableNameField
     - A boolean that indicates the user should not be able to edit the names of Attributes

       (Optional - default is *false*)

   * - DisableTopButtons
     - A boolean that indicates the top buttons (*New*, *Copy*, and *Delete*) should be disabled thereby preventing the user from creating or deleting Attributes.

       (Optional - default is *false*)

   * - HideAssociations
     - A boolean that indicates if the View GUI should not display association information.

       (Optional - default is *false*)

   * - RequireAllAssociated
     - A boolean that indicates if all Resource/Resource Components that can be associated to a specific type of Attribute should have an Attribute of that type associated with it in order to be considered *valid*.

       (Optional - default is *false*)

   * - AvailableLabel
     - A string that is used to indicate the Resources/Components that could be associated with the Attribute.

       (Optional - default is *Available*)

   * - CurrentLabel
     - A string that is used to indicate the Resources/Components that are currently associated with the Attribute.

       (Optional - default is *Current*)

   * - AssociationTitle
     - A string that is the title of the widget used deal with the Attribute's association information.

       (Optional - default is *Model Boundary and Attribute Associations*)

   * - DisplaySearchBox
     - A boolean that indicates if the user should be able to search Attributes by name.

       (Optional - default is *true*)

   * - SearchBoxText
     - A string that is used in the search box to indicate what the box is for.

       (Optional - default is *Search attributes...*)

   * - AttributeNameRegex
     - A string that is used to determine if the name the user entered for an Attribute is valid.

       (Optional - default is there is no constraint on the name of an Attribute)

The XML Element should have one child element called <AttributeTypes>.  The <AttributeTypes> element is composed of a set of <Att> Elements.  Each <Att> must contain an XML Attribute named **Type** which refers to an Attribute Definition type name.

The View will provide a drop-down list of all non-abstract smtk::attribute::Definitions that are specified (or derived from) the types mentions in the <AttributeTypes> Element.  When
the user selects one of the Definitions, it will display all of the existing Attributes that
match the type in the list view.

When an attribute is selected from the list, the view also displays editing fields for the items contained by the attribute, and an Association  widget for assigning the attribute to resource components. (The associations widget is only displayed if the corresponding Attribute Definition includes associations and the **HideAssociations** option is not set to *true*.)


Here in an example UI of an Attribute View.

  .. findfigure:: attributeViewExample.*
   :align: center
   :width: 90%

Group View
^^^^^^^^^^

.. list-table:: Alias and Type name for Group Views
   :widths: 10 40
   :header-rows: 1

   * - Alias
     - Type Name

   * - Group
     - smtk::extension::qtGroupView


A Group View represents a collection of other Views.  This collection
is laid out based on the **Style** attribute.  The supported styles are:

* tiled - each child view is displayed vertically in the Group View's Frame.
* tabbed - each child view is displayed in its own separate tab.
* groupbox - each child view is displayed vertically within its own collapsible Frame.

.. code-block:: xml

  <View Type="Group" Name="TopLevel" FilterByAdvanceLevel="true" TabPosition="North" TopLevel="true">
    <Views>
      <View Title="Set Analysis" />
      <View Title="Materials" />
      <View Title="Boundary Conditions" />
    </Views>
  </View>

Here is an example of a Group View with Style set to *Tabbed*.

  .. findfigure:: tabbedViewExample.*
   :align: center
   :width: 90%

Here is the same Group View with Style set to *Tiled*.

  .. findfigure:: tiledViewExample.*
   :align: center
   :width: 90%

Finally, here is the same Group View with Style set to *GroupBox*.  In this case
one of the children is opened and other is closed.

  .. findfigure:: groupBoxViewExample.*
   :align: center
   :width: 90%



.. list-table::  XML Attributes for Group Views
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Style
     - A string that indicates the style to be used to display the View's children Views.

       (Optional and Case Insensitive - default is *tabbed*)

   * - ActiveTab
     - A string that corresponds to a child view name.  If set and Style is tabbed, then
       the corresponding tab will be displayed.

       (Optional)

   * - TabPosition
     - A string that corresponds to which side the tabs of a tabbed Group View should be
       displayed (*north*, *south*, *east*, or *west*).

       (Optional and Case Insensitive - default is *north*)

This XML element consists of a single child element called <Views>.  The <Views> element contains a list of <View> XML Elements.

.. list-table::  XML Attributes for <View>
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Title
     - String value that refers to an existing View that will be displayed as a child to this one.

   * - Open
     - A boolean that indicates if the groupbox for that child view should be open or closed.

       (Optional - default is closed)

Instance View
^^^^^^^^^^^^^

.. list-table:: Alias and Type name for Instance Views
   :widths: 10 40
   :header-rows: 1

   * - Alias
     - Type Name

   * - Instance
     - smtk::extension::qtInstanceView


An Instance View will display a set of an Attributes for the user to edit.

.. code-block:: xml

    <View Type="Instanced" Title="General">
      <InstancedAttributes>
        <Att Name="numerics-att" Type="numerics"/>
        <Att Name="outputs-att" Type="outputs" />
        <Att Name="simulation-control-att" Type="simulation-control" />
      </InstancedAttributes>
    </View>

The XML Element should have one child element called <InstancedAttributes>.  The <InstancedAttributes> element is composed of a set of <Att> Elements.

.. list-table::  XML Attributes for <Att> Elements
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Name
     - A string that indicates the name of the Attribute.

   * - Type
     - A string that indicates the type of the Attribute.

   * - Style
     - A string that indicates the *Style* to be used to display the Attribute.

       (Optional)

Here in an example UI of an Associations View.

  .. findfigure:: instanceViewExample.*
   :align: center
   :width: 90%


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
