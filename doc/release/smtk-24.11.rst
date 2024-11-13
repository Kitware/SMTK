.. _release-notes-24.11:

=========================
SMTK 24.11 Release Notes
=========================

See also :ref:`release-notes-24.01` for previous changes.


SMTK Common Related Changes
===========================

Removing Default Role Type for Links
------------------------------------

Links will no longer have a default role type of undefinedRole since this lead to issues where the specified role was not being used.
This role type has been removed.

Resource based Links now use the same value to represent an Link with an invalid role type.
Also by default, links with invalid role types  will no longer be copied when a resource is copied.


SMTK Resource Related Changes
=============================

Copy API
--------

The :smtk:`Resource <smtk::resource::Resource>` class now has virtual methods to
produce an empty ``clone()`` of itself; to copy user data (via ``copyInitialize()``;
and to copy internal/external relationships among components (via ``copyFinalize()``).
See the user's guide for more information.

A new operation, :smtk:`CopyResources <smtk::operation::CoypResources>` is provided
that invokes these methods on a collection of resources.

Developer changes
~~~~~~~~~~~~~~~~~~

If you maintain your own resource type, you must override the methods above
if you wish to support cloning, copying, and updating your resources.
You may also wish to

* Implement copying of renderable geometry in your resource's custom
  subclass of :smtk:`smtk::geometry::Geometry`.
  By default, no geometry is copied.
  However, if you use the :smtk:`smtk::geometry::Cache` class for your resource's
  backend – as is commonly done for SMTK's resource classes – copying is implemented.
  Note that the Cache now requires the `DataType` type-alias in your backend to
  be copy-constructible.

* Review any custom property types registered for your resources;
  if they are not copy-constructible,
  :smtk:`smtk::resource::Resource::CopyProperties` will not copy
  them; either you must implement a copy constructor or copy these
  property values yourself.

User changes
~~~~~~~~~~~~

SMTK now provides a "copy resources" operation that accepts a set of input
resources to duplicate.
If all of the resources provide support for copying, then a copy of each
one is constructed and added to the resource manager on completion.

Template API
------------

The attribute-system methods to set and get a "template type" and "template version" have
been moved up to :smtk:`smtk::resource::Resource::templateType`
and :smtk:`smtk::resource::Resource::templateVersion` (along with their "set" methods).
See the user's guide for more information.


SMTK Attribute Related Changes
==============================

Attribute Definitions are now Resource Components
-------------------------------------------------

:smtk:`smtk::attribute::Definition` now derives from :smtk:`smtk::resource::Component`.
This means that Definitions now have UUIDs and may also have properties.

**Note** You do not need to specify UUIDs in either the XML or JSON file formats.  If an
ID is not specified, one will be assigned to it.

**Note** All previous JSON and XML files are supported by this change.  Version 8 XML and 7 JSON files and later
will support storing the IDs.  These formats will also support properties defined on Definitions.
As in the case for properties on the Resource and Attributes, the XML format only supports reading properties.

Developer changes
~~~~~~~~~~~~~~~~~~

** API Breakage: ** Since classes derived from the resource component class must provide a
method to return a shared pointer to a :smtk:`smtk::resource::Resource` instance via a member functions called
resource() and since the Definition class already had a method called resource() that returned a shared pointer
to its owning :smtk:`smtk::attribute::Resource`, this resulted in breaking API.  A new method called attributeResource()
was added to Definition that provides that same functionality as its original resource() method.  A simple name replacement is
all that is needed to resolve compilation errors resulting from this change.

:smtk:`smtk::attribute::Attribute::setId()` method was not being properly supported and now generates an error message if called.

The code used to parse property information in XML files has been relocated from the XMLV5Parser to its own file so it
can be reused.

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

Supporting Explicit Units for DoubleItems
-----------------------------------------

DoubleItems can now have units explicitly assigned to them provided that their Definition does not
specify units.  This allows Items coming from the same Definition to have different units.

Modified API
~~~~~~~~~~~~

* ``DoubleItemDefinition::hasSupportedUnits`` has been moved to ValueItemDefinition

Added API
~~~~~~~~~

* ValueItem
  * units() - returns the native units for the item
  * supportedUnits() - returns the supported units for the item.  If there is no Units System assigned to its definition or if its units are supported by the Units System, an empty string is returned else it returns its units.
* ValueItemDefinition
  * supportedUnits() - similar in concept as ValueItem's
* DoubleItem
  * setUnits() - explicitly sets the units of the item
  * units() - overridden to support explicit units
  * hasExplicitUnits() - returns true if the item has explicit units.

When changing the units of an Item, the system will see if the Item's current input string values are compatible, if they are not, the input value units are replaced with the new ones.

See smtk/attribute/testing/cxx/unitDoubleTest.cxx for an example.

Both XML and JSON formats have been updated to support this functionality as well as qtInputsItem.cxx, qtDoubleUnitsLineEdit{.h, .cxx}.

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

Definitions whose units are "*" indicate that derived Definitions and the Attributes instantiated from them may be assigned any supported units.

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

Changes to ItemPath Related Methods
-----------------------------------

Attribute::itemPath is being deprecated and replaced by Item::path.  Previously Attribute::itemPath did not properly handle Items that were contained within the sub-groups of a GroupItem; Item::path does.  Note that the deprecated does call Item::path under the covers.  The main reason for the deprecation was that the functionality only depends on the Item and forcing developers to get the Item's Attribute in order to get its path was not necessary and potentially costly

Attribute::itemAtPath was changed to be completely compatible with Item::path.  The original issues was the interpretation of the separator string parameter.  In the methods that return the path, the separator was inserted between the components of the path while in Attribute::itemAtPath, it was interpreted as a list of characters where any character in the string would be considered a separator.  This change forces Attribute::itemAtPath to use the interpretation of Item::path.

Also added a unit test called ``unitItemPath.cxx``.

Also updated qtInstanceView to use Item::path when referring to modified Items instead of just their names.  This makes it consistent with qtAttrubuteView.

Added ability to assign custom relevance functions to Items
-----------------------------------------------------------

SMTK now supports the ability to assign a function to an Attribute Item that would
be used to determine if an Item is currently relevant.  ``Item::isRelevant``
method has been refactored and most of the original logic has been moved to a new method called ``Item::defaultIsRelevant``.
To set a custom relevance function, use ``Item::setCustomIsRelevant``.

To determine if an Item is relevant you will still call ``isRelevant`` and it will use the custom relevance function if one has been set, else it will call the default method.

Please see ``smtk/attribute/testing/cxx/customIsRelevantTest.cxx`` for an example of how to use this functionality.

Added ability to assign custom enum relevance functions to Value Item Definitions
---------------------------------------------------------------------------------

SMTK now supports the ability to assign a function to a Value Item Definition that would
be used to determine if a Value Item's discrete enumeration value  is currently relevant.

Please see ``smtk/attribute/testing/cxx/customIsRelevantTest.cxx`` for an example of how to use this functionality.

Added `attribute::Resource::hasDefinition(type)` Method
------------------------------------------------------

This method will return true if the resource already contains a Definition with
the requested type.

Added `attribute::Analyses assignment method
--------------------------------------------

You can now assign the contents of one Analyses instance to another.  This will create copies
of all of the Analysis instances owned by the source Analyses.

Supporting Vector Properties in XML Attribute Templates
-------------------------------------------------------

Starting in Version 8 XML Template Files, you can now define vector-based Properties on Attribute Resources and Attributes.
Currently vector of doubles and vectors of strings are supported but this can be easily extended.

Here is an example and is available in the data/attributes/attribute_collections directory as propertiesExample.sbt.

.. code-block:: XML

  <SMTK_AttributeResource Version="8">
    <Properties>
      <Property Name="pi" Type="Int"> 42 </Property>
      <Property Name="pd" Type="double"> 3.141 </Property>
      <Property Name="ps" Type="STRING">Test string</Property>
      <Property Name="pb" Type="bool"> YES </Property>
      <Property Name="animals" Type="vector[string]">
        <Value>the dog</Value>
        <Value>a cat</Value>
      </Property>
    </Properties>
    <Definitions>
      <AttDef Type="Test"/>
    </Definitions>
    <Attributes>
      <Att Name="foo" Type="Test">
        <Properties>
          <Property Name="pi" Type="int"> 69 </Property>
          <Property Name="pd" Type="Double"> 3.141 </Property>
          <Property Name="ps" Type="String"></Property>
          <Property Name="pb" Type="Bool"> 1 </Property>
          <Property Name="pvd" Type="vector[double]">
            <Value>10.0</Value>
            <Value>20.0</Value>
          </Property>
        </Properties>
      </Att>
    </Attributes>
  </SMTK_AttributeResource>

Added Template Support When Defining Analyses
---------------------------------------------

SMTK XML Attribute File now supports instantiation of templates inside the Analyses
XML Element Block.


SMTK Operation Related Changes
==============================

Deprecated Key struct
---------------------

The ``Key`` struct has been deprecated in favor of ``BaseKey`` which contains options to alter
certain behaviors of ``Operation::operate()`` when running an operation from within another
operation's ``operateInternal()`` synchronously.

``Key`` derives from ``BaseKey`` so that the legacy API can still be satisfied, but any dependent
code should be refactored to use ``Operation::childKey()`` instead

See the user documentation for more details about the options that can be passed to
``Operation::childKey()`` in the "Resource Locking" section.

Proper GIL locking
------------------

The scheme SMTK uses for running operations from python
has changed to address potential deadlocks.
This should not require any changes to code that depends
on SMTK and may fix issues with your python operations.

PyOperation methods must not have ownership of the GIL since
the ``PYBIND11_OVERLOAD`` macro acquires it (resulting in a deadlock
if the GIL is already held). However, PyOperation instances may
be called from either C++ or from Python. If called from Python,
we must release the GIL held by python so it can be acquired.
If called from C++ code, we must not release the GIL since we
do not already hold it.

Instead of always releasing the GIL inside PyOperation, release
it from within the python bindings. This way, we do not release
the GIL when it is not held (causing assertion failures on macos
and perhaps other platforms).

In addition to releasing and then re-acquiring the GIL when
calling python operation code from python, we must ensure
that any python code is executed on the main application thread
(i.e., the GUI thread) even when invoked from a separate thread.
This way, python operations can be launched in a separate thread
that yields until all the required resources are locked and then
run in the GUI thread with the proper GIL locking.

SMTK Changes to Graph Resource
==============================

Need to Override Clone Method
--------------------------------

An issue with the :smtk:`smtk::graph::Resource` template's ``clone()`` method
has been identified. When the source resource does not override ``clone()`` and
does not have a resource-manager, the base ``clone()`` implementation will no
longer attempt to create a resource. If you encounter this situation, you are
now expected to override the ``clone()`` method in your subclass and call the
new ``prepareClone()`` method with the resource you create.


SMTK Task Related Changes
================================

Task Agents
-----------

Tasks have been refactored to use "agents" to manage their state. This allows tasks to compose behaviors of multiple agents. We have implemented 3 agents for this release:

* GatherObjectsAgent – broadcast a pre-configured (or programmatically-configured) set of objects on a parent-task’s output port.
* FillOutAttributesAgent – check that attributes a user may edit are valid (i.e., all items have proper values and associations are allowable).
* SubmitOperationAgent – ensure a user has run an operation at least once. Note that unlike the SubmitOperation task (with no agents), the agent functions as both the ConfigureOperation adaptor and the SubmitOperation task; it can configure operation parameters with values taken from input ports. Also, it broadcasts resources mentioned in the most recent operation's run on its output port.

See the user's guide for more information on how to configure agents.

Ports
-----

The concept of :smtk:`task ports <smtk::task::Port>` has been introduced to
pass configuration data among tasks.
See the task-system :ref:`smtk-task-concepts` section of the user-guide for
more information.

Renamed Method in FillOutAttributes Task
----------------------------------------
* hasRelevantInfomation has been renamed hasRelevantInformation and the original method has been deprecated.

SMTK View Related Changes
=========================

Added view::Configuration::Component::attributeAsString method
--------------------------------------------------------------

Added a method that returns the string value of a component's attribute.
If the attribute doesn't exist an empty string is returned.

Expandable Multi-line String Items
----------------------------------

You can now indicate that you want a Multi-line StringItem to have an expanding size policy
for its height by using the new **ExpandInY** option.  Here is an example:

.. code-block:: xml

  <Views>
    <View Type="Instanced" Title="StringItemTest" Label="Simple String Item Test" TopLevel="true">
      <InstancedAttributes>
        <Att Type="A" Name="Test Attribute">
          <ItemViews>
            <View Item="s" ExpandInY="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>

Please see data/attribute/attribute_collection/StringItemExample.sbt for a complete example.



SMTK UI Related Changes
=======================

Double-item with unit completion
--------------------------------

The ``qtDoubleUnitsLineEdit.cxx`` class now uses the unit-library's
``PreferredUnits`` class to suggest unit completions. This allows
workflow developers to provide a tailored list of suggested completions
rather than listing every available compatible unit.

Line Edit widget with unit completion
-------------------------------------

The ``qtUnitsLineEdit.cxx`` class is for entering strings that represent units.
The widget provides a units aware completer as well as color coding its background
based on the entry being a valid unit or if the unit is the default.
as in the case of  ``qtDoubleUnitsLineEdit.cxx`` class, it can make use of the unit-library's
``PreferredUnits`` class to suggest unit completions. This allows
workflow developers to provide a tailored list of suggested completions
rather than listing every available compatible unit.

Displaying Attribute with Units
-------------------------------
Using the new ``qtUnitsLineEdit`` class, end-users can now set units on an Attribute (assuming that its Definition supports units).
This required changes to ``qtAttribute`` to not consider an Attribute *empty* if it had no items to display but did have specified units.

New XML Attribute for controlling how Units are displayed
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can use UnitsMode to indicate how an Attribute's units should be displayed.

* none - do not display the attribute's units
* viewOnly - display but do not allow the units to be changed
* editable - allow the user to edit the attribute's units

**Note** that these values are case insensitive

See DoubleItemExample.sbt as an example that demonstrates this new functionality.

Changes to ``qtBaseAttributeView`` and Derived View Classes
------------------------------------------------------------

* Added displayAttribute method that returns true if the attribute should be displayed based on its relevance.
* Changed displayItem to take in a ``const smtk::attribute::ItemDefinitionPtr&`` instead of a ``smtk::attribute::ItemDefinitionPtr`` **Note** that this does break API though it is very simple to update to the new API

Custom TaskNode Context Menus
-----------------------------

Added a virtual method to qtBaseTaskNode to setup a context menu for a qtTaskNode.

Ability to Hide the Legend in qtDiagram
---------------------------------------

You can now force the arc-type legend to be hidden in the view configuration for
your diagram; simply set the ``Legend`` attribute to be false.
See the default panel configuration (``smtk/extension/qt/diagram/PanelConfiguration.json``)
for an example of where the ``Legend`` attribute should be placed.

qtDiagram Related Bug Fixes
---------------------------

+ The `qtDiagramGenerator::updateScene()` method has been split into
  `qtDiagramGenerator::updateSceneNodes()` and `qtDiagramGenerator::updateSceneArcs()` so that arcs
  between nodes in different generators can be maintained properly. The diagram will call every
  generator's `updateSceneNodes()` method before calling generators' `updateSceneArcs()` methods.
  This way, creation of arcs can be delayed until all of the generators have created all new nodes.
  You should split your custom generator's `updateScene()` method into these two methods.
  Old code will continue to compile but will generate a runtime error message.
+ A crash when removing task nodes from ``qtTaskEditor`` was fixed.
+ The escape key now works to return to the default interaction mode
  from all of the ``qtDiagramViewMode`` subclasses.
+ Deleting arcs can be accomplished with either the delete or backspace
  keys (previously, only backspace worked).
+ Make shift in "select" mode temporarily enter "pan" mode to mirror
  the behavior of the same key in "pan" mode (which enters "select").
+ Replaced GridLayout which was causing the widget not to expand to consume all of the available space.


SMTK ParaView Extension Related Changes
=======================================

3-d widget operation observer
-----------------------------

The :smtk:`pqSMTKAttributeItemWidget` class was monitoring operations to determine
when the attribute holding its items was modified.
It should not do this, instead relying on qtAttributeView or qtInstancedView to
call its ``updateItemData()`` method when an operation modifies the widget.
(It was already doing this.) Because its operation observer was not validating
that the operation causing changes was triggered by itself, a race condition
existed that could undo changes to one item controlled by the widget while
parsing updates from another item.


SMTK Third-Party Related Changes
================================

ParaView Support
----------------

This release of SMTK works with ParaView 5.13.1.
