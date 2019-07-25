# SMTK 3.2 Release Notes
SMTK 3.2 is a minor release with new features.  Note any non-backward compatible changes are in ***bold italics***. See also [SMTK 3.1 Release Notes](smtk-3.1.md).

## Introduce Archive Utility

An SMTK Archive is a portable collection of files that are stored as a
single file on-disk. An archive is described by its filesystem path. Once
instantiated, a user can insert files into the archive,
serialize/deserialize the archive to/from disk, access a list of files in
the archive, and acquire file streams to these files by accessing them via
their name. An archive can be considered a directory containing files;
as such, each file in the archive must be assigned a unique path.

## Changes to Attribute Resource
### Changes to representing Analyses
Analyses are now represented by their own class instead of a collections of maps and vectors managed directly by the Attribute Resource.  The smtk::attribute::Analyses class manages a collection of smtk::attribute::Analyses::Analysis objects.

It is relatively simple to convert to the new design:

* smtk::attribute::Resource::analyses() will now return a reference to the analyses object in the resource.  All analysis-based operations now go through it
* To get all analyses - note that the return type is different!
	* Old Method
		* resource->analyses()
	* New Method
		* resource->analyses()
* To get the number of analyses
	* Old Method
		* resource->numberOfAnalyses()
	* New Method
		* resource->analyses().size()
* To create a new analysis
	* Old Method
		* resource->defineAnalysis(name, categories)
	* New Method
		* Analyses::Analysis a = resource->analyses().create(name)
		* a->setLocalCategories(categories)
* To set the parent of an analysis
	* Old Method
		* resource->setAnalysisParent(name, parentName)
	* New Method (2 ways)
		* Without getting the analysis object
			* resource->analyses().setAnalysisParent(name, parentName)
		* Through the analysis object
			* Analyses::Analysis a = resource->analyses().find(name)
			* Analyses::Analysis p = resource->analyses().find(parentName)
			* a->setParent(p) <-- if a and p are not nullptr
* To get the parent of an analysis
	* Old Method
		* resource->analysisParent(name)
	* New Method
		* Analyses::Analysis a = resource->analyses().find(name)
		* a->parent() <-- if a is not nullptr
* To get the children of an analysis
	* Old Method
		* resource->analysisChildren(name)
	* New Method
		* Analyses::Analysis a = resource->analyses().find(name)
		* a->children() <-- if a is not nullptr
* To get the local categories associated with an analysis
	* Old Method
		* resource->analysisCategories(name)
	* New Method
		* Analyses::Analysis a = resource->analyses().find(name)
		* a->localCategories() <-- if a is not nullptr
* To get the all the categories associated with an analysis including those inherited from its parent analysis
	* Old Method
		* none
	* New Method
		* Analyses::Analysis a = resource->analyses().find(name)
		* a->categories() <-- if a is not nullptr
* To get the top level analyses (those without parents)
	* Old Method
		* resource->topLevelAnalyses(name)
	* New Method
		* Analyses::Analysis a = resource->analyses().topLevel()
		* a->children() <-- if a is not nullptr
* To build an Analysis Definition representing the analyses
	* Old Method
		* resource->buildAnalysesDefinition(typeName)
	* New Method
		* Analyses::Analysis a = resource->analyses().buildAnalysesDefinition(resource, typeName)

### Exclusive Analyses
With the above changes also comes a way to indicate if an analysis' children represent analyses that can be combined by selecting a subset of them or if exclusively if only should be selected.  You can also indicate if the top level analyses are suppose to be exclusive with respects to each other.

```xml
  <Analyses Exclusive="true">
    <Analysis Type="Heat Transfer" Exclusive="true">
      <Cat>Heat Transfer</Cat>
    </Analysis>
    <Analysis Type="Enclosure Radiation" BaseType="Heat Transfer">
      <Cat>Enclosure Radiation</Cat>
    </Analysis>
    <Analysis Type="Induction Heating" BaseType="Heat Transfer">
      <Cat>Induction Heating</Cat>
    </Analysis>
    <Analysis Type="Fluid Flow">
      <Cat>Fluid Flow</Cat>
    </Analysis>
    <Analysis Type="Solid Mechanics">
      <Cat>Solid Mechanics</Cat>
    </Analysis>
  </Analyses>

```
### Analysis Labels
As with other objects stored with an Attribute Resource, an Analysis can have an optional label associated with it.  Analysis::displayedName() will return the analysis' label if its label has been set, else it will return its name.  This method is used mainly for displaying the Analysis within a UI (such as an Analysis View).

### Change to RemoveValue Methods for Items Containing Values
Prior to this release a removeValue method would return false if the code attempted to either remove a value from a non-extensible item or if removing a value below the item's required number of values.  The new behavior is as long as the index passed into the method is valid, the method will return true.  In the case the index is < the item's number of required values, the method acts as the item's unset method.  In the case the index >= number of required values but < the item's number of values, the storage for the value itself is removed and the number of values decreased.

### New Methods added to attribute::Resource
* hasAssociations() - returns true if resources have been directly associated to the attribute Resource (even if the resources are not loaded in memory)

### Restrict associations to resources

Attribute associations now have the boolean attribute "OnlyResources"
that, when set to true, restricts associations to only resources.

### Item Rotate Feature

Two new methods were added for rotating the elements contained in attribute items:

    bool smtk::GroupItem::rotate(
      std::size_t fromPosition, std::size_t toPosition)

    bool smtk::ValueItemTemplate::rotate(
      std::size_t fromPosition, std::size_t toPosition)

(Note: ValueItemTemplate is superclass to DoubleItem, IntItem, and StringItem.)

Each method takes in two std::size_t arguments and moves the element at
the first position argument to the position indicated by the second argument,
and shifts the remaining elements accordingly. The method returns true if
the rotation was successfully applied. The rotation is not applied if either
position argument is outside the range of the item's data, or if the
position arguments are equal to each other (which would be a no-op).

Potential use for these methods are to support drag and drop operations
in user interface code.

### Reference Items do not hold references by default

Originally, ReferenceItems cached shared pointers to resources and
components. The use of shared pointers resulted in the undesired
behavior of resources outliving their presence in the resource
manager. There is now an option for ReferenceItems (disabled by
default) to use shared pointers to store their references. By default,
ReferenceItems now use weak pointers, allowing the referenced
resource/component to go out of scope.


## Changes to Mesh Resource Support
### Updates for mesh visualization

vtkMeshMultiBlockSource has been updated to index instances of
smtk::mesh using UUIDs, allowing for a significant amount of code
reuse between smtk::mesh and smtk::model.

### Disable Moab STL Importer

An update to MOAB's lastest master caused the stl importer to fail. Until
this is fixed, we temporarily disable MOAB's stl reader (we still have
VTK's stl reader, if VTK is enabled).

### Additional mesh extraction routines

We have added a new mesh utility named
`extractCanonicalIndices`. This functionality is useful for exporting
to mesh formats that describe mesh faces by the index and face ID of
the 3-D cell containing the face. Given a (D-1)-dimensional mesh whose
(D)-dimensional adjacencies are held in a (D)-dimensional reference
mesh, the utility will return the extracted cell index of the
reference mesh and the canonical index of the (D-1)-dimensional cell
with respect to its (D)-dimensional adjacency. The canonical index of
a cell is defined in Tautges, Timothy J. "Canonical numbering systems
for finite-element codes." International Journal for Numerical Methods
in Biomedical Engineering 26.12 (2010): 1559-1572.


## Changes to Operation Support
### Add a group for declaring operations internal

By default, any operation that is registered with an operation manager
will be presented in the list of available operations retrieved by the
AvailableOperations class. An operation group,
smtk::operation::InternalGroup, has been added so developers can
explicitly mark their operation as an internal, or private,
operation. Operations added to the InternalGroup will not be presented
to the user as available.

### Hide advanced items in operation view for "New Resource" menu option

Currently, creation operators all fallow the pattern of optionally creating
an entity within an extant resource. Since this functionality doesn't make
sense for a "New Resource" menu option, we flag the input items involved
with that choice as advanced and disable the advanced items when the
operation is launched via the "New Resource" menu option. Since the
choice of filtering by advance level is persistent for the operation, we
unset the flag after the operation window returns.

### Operations to read and write mesh resources as smtk files

All SMTK resources have the ability to read to and write from a json
file to pickle their state. Since the native file format for the dat
of an SMTK mesh is a .h5 file, we have introduced operations that
read and write SMTK mesh resources as a json description that includes
a URL to the underlying .h5 file.

### Import/Export Attribute Resource Operations

smtk::attribute::Import and smtk::attribute::Export operations import
and export attribute resources to/from XML-based SimBuilder files
(*.sbt, *.sbi).

### Improvements to thread safety for operations

qtOperationLauncher has been updated to fix a bug where operation
results were not correctly propagated to the right Qt
signals.

Operations now lock their specifications whenever they interact with
them. We have also added a thread-safe means of generating unique
names for parameters and results.

### Updates to operation's Python API

The input parameters for SMTK's operations are described using an
instance of smtk::attribute::Attribute. To improve the API for
manipulating operation parameters in Python, we now generate
additional methods for setting & accessing operation parameters using
more intuitive method names. For example, If there is a
double-valued input to an operation with the name "foo", the Python
API for accessing this field has changed from

```
  operation.parameters().findDouble('foo').setValue(3.14159)
  foo = operation.parameters().findDouble('foo').value()
```
to

```
  operation.parameters().setFoo(3.14159)
  foo = operation.parameters().foo()
```

The original Python API for manipulating operation parameters is still
valid; this change simply introduces additional API to the same data.


## Changes to the View System and Qt Extensions

### AttributeItemInfo has been renamed
The new name is qtAttributeItemInfo and it is now located in its own header and cxx files instead of qtItem.h and qtItem.cxx.  **To update existing code all you need do is a name replace.**  qtItem.h includes qtAttributeItemInfo.h so no additional includes are needed.

### Supporting Item Paths for ItemViews
You can now refer to an item via its path relative to its owning Attribute and uses UNIX style.  Item names that could be used as variable names in C++ are supported and have been extended to also include . and -.  The following show several examples:

```xml
  <Views>
    <View Type="Instanced" Title="Grammar Test" TopLevel="true">
      <InstancedAttributes>
        <Att Name="Test Attribute" Type="test">
          <ItemViews>
            <View Item="a" Type="Default" Option="SpinBox"/>
            <View Path="/b" ReadOnly="true"/>
            <View Path="/c/d" FixedWidth="20"/>
            <View Path="/c/e" Option="SpinBox"/>
            <View Path="/f/f-a" Option="SpinBox"/>
            <View Path="/f/f-b" FixedWidth="10"/>
            <View Path="/f/f-c/f.d" FixedWidth="0"/>
            <View Path="/f/f-c/f.e" Option="SpinBox"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
```
In the case of extensible Group Items, the item view style refers to subgroups.  For a complete example, please see viewPathTest.sbt that is located in data/attribute/attribute_collection.

### Changes to qtItem
* **Removed passAdvanceCheck method** - In the qtItem classes, to determine if an item should be viewed, the code was either calling qtBaseView::displayItem check or a combination of qtUIManager::passItemCategoryCheck and this method.  Since a view can have its own set of filtering rules, having this method just confuses the developer as to how to determine if the item should be displayed.
* Added removedChildItem(..) method - this removes a child qtItem from the object and calls deleteLater() on it.
* **addChildItem(..), clearChildItems() and childItems() methods are now protected.**  They use to be public which was a mistake.

### Changes to qtFileItem
* Invalidity calculation now takes into consideration the ShouldExists hint associated with the item's definition.
* Combobox now only sets the item's value when the user finishes entering the filename instead of every keystroke.

### Changes to qtInputItem
* Added a forceUpdate method - this always forces the object to act as if the underlying item was modified.  Used mainly by helper classes like qtDiscreteValueEditor.

### Added qtReferenceItemComboBox
This is a new type of qtItem used to create a simple ComboBox UI for setting a qtReferenceItem.  It also supports the ability to restrict its possible values to those objects associated with the item's attribute.

### Changes to qtDiscreteValueEditor
* The modified signal from the corresponding qtInputItem is no longer sent when the underlying ValueItem is modified.  It is now sent after the Editor's internal widgets have been appropriately updated.

### Changes to qtBaseView
* The displayItem test now calls 2 new methods categoryTest and advanceLevelTest.  This makes it easier for derived classes to override the filtering behavior

### Changes to qtAnalysisView
* Now supports Exclusive property for Analysis and Top Level Analyses
* The order of displaying analyses is no longer resorted alphabetically and will be displayed in the order they were defined.
* Overrides the categoryTest method to return always true since its data should never be filtered based on categories.

### Changes to qtUIManager
* The set of categories is being passed to setToLevelCategories is now compared to what was already set.

### Changes to qtAssociationWidget
* Added the ability to ignore a resource when determining which objects can be associated with an attribute.  The main use case is when refreshing the widget because a resource is about to be removed from the system.  We don't want it to contribute to the calculation.

### Displaying Operations in the Operation Manager
* Operations will now always appear sorted
* Operations will have a tooltip associated with it based on it's brief description

### Descriptive Phrase Changes

+ Changes to component names did not always result in
  reordering of phrases in the resource tree-view.
+ Reordering of phrases could result in a crash due
  to improper offsets passed to Qt's item model.
+ Now `smtk::view::PhraseContent` has a new internal
  variable to control the mutability of the top-level
  phrase's aspects (title, color, etc.). By default,
  everything is marked mutable (fixing a bug in
  ComponentPhraseModel), but you may change this by
  calling `setMutableAspects()` with a bit-flag taken
  from the values in the `PhraseContent::ContentType`
  enumeration.

### Bug Fixes
* qtAnalysisView, qtAttributeView, qtInstancedView, qtModelEntityView and qtSelectorView now properly deletes any qtAttributes they create
* qtGroupItem now properly delete any children qtItems it creates.  It was only deleting children if it was extensible.

## Changes to SMTK's ParaView extensions

+ There is now a toolbar button to start selecting components
  (in ParaView parlance, this is a block selection).
  The button is in SMTK's selection-filter toolbar at the far
  right. Clicking it will start a block selection in the currently
  active view.

## Changes to SMTK's utilities

+ Some python scripts have been added to `utilities/lldb`
  for tracing shared-pointer references. See the readme
  in that directory for more information.

# Bug Fixes

* GroupItem::find - crashes when the group has no subgroups (Issue #245)
* Copying Discrete StringItem with no default value no longer crashes
* Order Dependency Issue - The issue was observed in ModelBuilder where closing a resource that is associated with an attribute resource and then reloading the resource.  The issue was that the associations were not correct and pointed to the closed resource.
