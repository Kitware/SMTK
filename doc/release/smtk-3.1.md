#SMTK 3.1 Release Notes
SMTK 3.1 is a minor release with new features.  Note any non-backward compatible changes are in bold italics.

##Changes to SMTK's Resource System
* isNameSet method was not properly returning the status of the Resource's name.  This has been fixed

##Changes to Attribute Resource System
###Adding Exclusions and Prerequisites
Attribute Definitions can now provide mechanisms for modeling exclusion and prerequisites constraints.  An exclusion constraint prevents an attribute from being associated to same persistent object if another attribute is already associated and its definition excludes the attribute's type.  For example, consider two attribute definitions A and B as well as two attributes a (of type A) and b (or type B).  If we have specified that A and B excludes each other then if we have associated a to an object, we are prevented from associating b to the same object.

In the case of a prerequisite, an attribute is prevent from being associated if there is not an attribute of an appropriate type associated with the object.  So continuing the above example lets assume we add a new definition C that has A as it prerequisite and an attribute c (type C).  c can be associated to an object only if a is already associated to it.

In addition, these properties are also inherited.  So if we derive type A1 from A in the above example, an attribute a1 (from type A1), would excludes b and would be considered a prerequisite of c.

**NOTE - the exclusion property should be symmetric (if A doesn't allow B then B shouldn't allow A) but the prerequisite property can not be symmetric (else neither could be associated - if you need to have both attributes always assigned together then the information should be modeled as a single definition)**

To implement this, the following was added to Definition:

* Set of excluded definitions
* Set of prerequisite definitions
* Added methods to add and remove definitions - when adding an exclusion, it is done symmetrically. So calling A->addExclusion(B) will result in B being added to A's exclusion set and A being added to B's exclusion's set.
* Added methods for checking association rules, exclusions, and prerequisites
* Added canBeAssociated method for testing persistent objects
* I/O classes can read and write these rules in XML and JSON


In addition, the implementation of isUnique has been changed  It now used the exclusion mechanism by inserting itself into the list.  Note that this rule is not written out when serialized or when saved to a file


The qtAssociationWidget has been modified to use these new ruled when determining availability and qtAttributeView has been modified to test for association rules instead of association masks


See attribute/testing/cxx/unitAttributeAssociationConstraints.cxx for an example.

###Changes to Attribute Association Related API

* attribute::Attribute
 * ***Removed functionality to maintain model::resource's attribute association back-store (no longer needed)***
 * Added a protected method forceDisassociate that will bypass disassociation checks.  This is used by the attribute::Resource when disassociating all attributes from an object.
 * Added association checks to the associate method.
* attribute::Resource
 * Added hasAttributes method to check to see if an object has attributes associated to it
 * Added disassociateAllAttributes method to remove all attribute associations from an object
* model::Entity
 * ***Removed functionality to maintain model::resource's attribute association back-store (no longer needed)***
* model::EntityRef
 * ***Removed functionality to maintain model::resource's attribute association back-store (no longer needed) and replaced it with link-based functionality***
 * Added hasAttributes(smtk::attribute::ConstResourcePtr attRes) const
 * Added disassociation methods that don't take in the reverse bool parameter.  The original API which does take in the reverse parameter is marked for depreciation (via comment) and calls the new API

###General Changes
* Added the concept of Parent Analysis.  The parent relationship is used to determine the categories associated with an analysis.
* Added the concept of Top Level Analyses - these are analyses that do not have a parent.
* Added the ability to create an attribute definition to represent the analysis structure.
* IO (both JSON and XML) have been changed to support Analysis Parent Relationships.

## Changes to Model Resource System
### Resource and Entity API

+ The model resource's `queryOperation()` method is now implemented in
  Entity.cxx and includes the ability to filter entities by their type
  bits (previously available) and their property values (new functionality).

### Operations

+ The "assign colors" operation now provides a way to set opacity independently
  of or in tandem with colors. The user interface takes advantage of this to
  provide an opacity slider. Since all model-entity colors have been stored as
  a 4-component RGBA vector, the model representation now properly sets block
  opacities.

### PyBind11
* Added Pybind11 Registrar methods for smtk::model


## Changes to Mesh Resource System
* Now that `smtk::resource::Resource` holds a separate name
  (i.e., independent of location), remove the API that mesh
  resources provided to avoid duplication.

## Changes to SMTK's Operation System

### Configuration

Operations now have a method named `configure()`.
This method may be invoked by a user interface when
the operation's associations are changed, item values are
changed, or new attributes are created (in the operation's
resource) in order for the operation to edit itself
for consistency and to provide context-sensitive default
values for items.

An example is provided in the oscillator-session's EditSource
operation that uses changes to associations to compute a default
center and radius for the source.

### Introduce a thread pool for multithreaded operation execution

To prevent applications from appearing to "hang" when long-running operations
are executed, a thread pool has been introduced for the managed
launching of operation tasks. The use of a thread pool allows for a
finite number of threads to be continuously reused for subsequent
operations, eliminating the overhead and potential bottleneck of
spawning a new thread for each operation.


##Changes to I/O
###Track attribute resource id, associations in xml I/O

In order for an attribute resource to be reliably written/read to/from
XML, its ID is now stored in its generated .sbi file. Additionally, an
attribute's associations are stored in XML with enough information to recreate their underlying links.

##Changes to SMTK's Observer Mechanism
### Add logic in the PV layer to force observers to fire on main thread

Qt requires that all methods that affect the GUI be performed on the application's main thread. Many of the registered Observer functions for both operations and resources are designed to affect the GUI. Rather than connect GUI-modifying logic to a signal triggered by an observer, we mutate the behavior of the operation and resource Observer calling logic to ensure that all Observer functors are called on the main thread, regardless of which thread performs the observation.

To support this pattern, SMTK's Observer pattern has been generalized to a single description (smtk::common::Observers) that adopts a run-time configurable type of polymorphism where consuming code can change the class's behavior, allowing consuming code to redefine the context in which the Observer functors are executed.

## Changes to the View System and Qt Extensions

### New View Type - Associations

This view has the same syntax as an Attribute View but only allows the user to change the association information of the attribute resulting in taking up less screen Real Estate

### New View Type - Analysis
An Analysis View is a specialized view for choosing the types of analyses the user wants to perform.  These choices are persistent and can be used by an export operation instead of having the operator ask the user what types of analyses should be performed.

Unlike other views the Analysis View will construct both an Attribute Definition and corresponding Attribute when needed.  The Attribute Definition is based on the Analysis Information stored in the Attribute Resource.  Any Analysis that is referred to by another will be represented as a Group Item.  All other Analyses will be represented as a Void Item.

The View also controls which categories are permitted to be displayed and/or selected.  The set is union of all of the selected Analyses' categories.

The following is an example of a Analysis View:

```xml
 <View Type="Analysis" Title="Analysis" AnalysisAttributeName="truchasAnalysis"
 AnalysisAttributeType="truchasAnalysisDefinition">
</View>
```

  * AnalysisAttributeType is the name of the Attribute Definition the view will create to represent the Analysis Structure (if needed)
  * AnalysisAttributeName is the name of the Attribute the view will create to represent the Analysis  (if needed)

### Changes to BaseView

* Added the concept of top level categories that represents a set of categories (that can be a subset of those defined in the attribute resource) that can be used to display or filter attribute information.
* The `<DefaultColor>` and `<InvalidColor>` configuration tags are no longer supported by qtBaseView.
  They have been turned into user preferences rather than XML/JSON view parameters.

###Changes to Attribute View

* added a new XML attribute "HideAssociations".  If set to true the view will not display the association editing widget save screen Real Estate
* If there is only one type of attribute being created/modified then the type column is no longer displayed
* For the time being the view by property  feature has been disabled until we can decide on whether it is useful and if so, what is the best way to display the information.
* The column "Attribute" has been renamed to "Name"
* Attempting to rename an attribute to a name already is use now generates a warning dialog.

###Changes to Group View

* View no longer displays empty tabs
* Current tabs are now remembered when the group rebuilds its widget - previously this was only true for the top-level tabbed group views
* Fixed issue with displaying Discrete Items that has children

###Changes to UIManager

* Added the ability to enable/disable category filtering
* Added support for top-level categories
* Colors to indicate default and invalid values are now Qt properties
  to support ParaView's user preference framework.
* The UI manager now emits a `highlightOnHoverChanged(bool)` signal
  when the user preference changes.
  The pqSMTKAttributePanel delivers these changes to the UI manager it maintains;
  other users of qtUIManager are expected to do the same.

### Item Changes

#### ReadOnly View Items

Added a new ReadOnly Option to Item Views.  In the following example the item, absolute-zero, in the attribute physical-constants has been made read only.  The current implementation disables the widgets defined by the read only item from being  modified but will still display them.

```xml
    <View Type="Instanced" Title="Global Constants">
      <InstancedAttributes>
        <Att Name="physics" Type="physics">
          <ItemViews>
            <View Item="fluid-body-force" Type="Default" FixedWidth="0"/>
          </ItemViews>
        </Att>
        <Att Name="physical-constants" Type="physical-constants">
          <ItemViews>
            <View Item="absolute-zero" Type="Default" ReadOnly="true" FixedWidth="50/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
```
####Changes to Displaying Items
* Extensible Group Items now display "Add Row" instead of "Add sub group" - this label can be changed using an ItemView with the XML attribute : ExtensibleLabel
* Added a FixedWidth Option for String, Double and Int ItemViews as also shown in the above example - **Note: setting the fixed width to 0 means there is no fixed width.**
* qtReferenceItem now allows developers to override the visibility icons
  with custom URLs. See qtReferenceItem::setSelectionIconPaths() and
  doc/userguide/attribute/file-syntax.rst for details.

####Highlighting when Hovering
* Views showing associations will now highlight geometry when the user hovers over it

#### New ViewItem Styles
* There are now Infinite Cylinder and Conical Frustum options for Group Items - See Changes to ParaView Extensions for more info.

#### Using QPointer it qtItem and AttributeItemInfo
* ***AttributeItemInfo::parentWidget() now returns a QPointer\<QWidget>***
* ***qtItem::parentWidget() now returns a QPointer\<QWidget>***
* ***qtItem::widget() now returns a QPointer\<QWidget>***

These changes were done to improve general stability as well as to simplifying when a qtItem needs to delete its main widget.

##Changes to Selection
* Selections now have a `resetSelectionBits()` method
  that provides a way to remove a bit-vector from
  all of the objects in the current selection map.

##Changes to Rendering
### Fix glyphing wth wrong scale and resurrect create instance

Now smtk will glyph prototypes with a per point scale array(default to [1,1,1])
instead of the magnitude.

## Changes to ParaView Extensions

### Box Widget

The box widget now accepts a single DoubleItem (with 6 entries)
specifying an axis-aligned bounding box or a GroupItem
containing multiple DoubleItems that configure a bounding box
in different ways depending on how they are used.
See the pqSMTKBoxItemWidget header for details.

### Infinite Cylinder Widget
There is now an infinite cylinder widget, which can be bound to a Group
item containing 3 Double children that serve as a center point,
a direction vector, and a radius.
Note that the widget models a cylinder of infinite length cut
by a bounding box whose size is not currently specified.

### Cone-Frustum Widget
There is now a cone-frustum widget which can be bound to a Group
item containing 3 or 4 Double children that server as endpoints
and endpoint radii. The cone has a special case for cylinders where
only 1 radius value is provided.

Create instace operation is made to be SMTK3 compatible.

## Changes to Simulation Namespace
* Added UserData classes to represent Ints, Doubles, and Strings

##Changes to Project Manager
### Add second model as an option when specifying new projects

The project manager was updated to accept a second geometry
file when initializing new projects. In this implementation,
only the primary geometry is linked to the simulation
attributes resource when the project is initialized.
(Resource links can be added or removed by application code,
of course.)


##Changes to Software Process
###Add cmake logic to generate a plugin config file

ParaView-derived applications currently ingest plugins in one of two
ways: the plugins are either linked directly into the application, or
they are loaded at runtime. For the latter case, plugin discovery is
performed by reading xml files that describe the plugin's name,
location and whether or not it should be automatically loaded.

When SMTK (and SMTK-derived) plugins are intended to be loaded at
runtime, it is convenient to have the plugin config file generated
using the CMake target properties from the plugins themselves. This
change introduces the exported plugin target property
`ENABLED_BY_DEFAULT` and adds the CMake function
`generate_smtk_plugin_config_file` for ParaView-derived applications
that consume SMTK plugins to generate a plugins file.

When SMTK (and SMTK-derived) plugins are intended to be directly
linked into an application , it is convenient to have a plugin library
generated using the CMake target properties from the plugins
themselves. This change adds the CMake function
`generate_smtk_plugin_library` for ParaView-derived applications
that consume SMTK plugins to generate a plugin library.

###Add mechanism to include plugin contract tests to an SMTK build

We introduce the configure variable `SMTK_PLUGIN_CONTRACT_FILE_URLS`, a
list of URLs for contract files that each describe a plugin as an external
project. For each file in this list, a test is created that builds and
tests the plugin against the current build of SMTK. An example plugin
contract file, <CMAKE_SOURCE_DIR>/CMake/resource-manager-state.cmake,
has also been added as a prototype file for plugin contract tests.

## Changes to SMTK Sessions
### Reactor Geometry Generation (RGG) Session has been moved into its own repository as a plugin

SMTK's RGG session has been moved into
[its own repository](https://gitlab.kitware.com/cmb/plugins/rgg-session)
as a plugin. Its development will proceed from this location.
