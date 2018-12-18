# SMTK 3.0 Release Notes

SMTK is a major release with many new features and accompanying API changes.
There are also a number of conceptual changes aimed to make SMTK more self-consistent.

## Conceptual changes

### Persistent objects, resources, and components

The major subsystems that deal with persistent data (attribute, model, and mesh)
have been refactored to inherit a common class, `smtk::resource::PersistentObject`.
This class has 2 immediate subclasses: `Resource` and `Component`.
Resources represent files (or other units of persistent storage) that contain components.

Each subsystem now has subclasses of the resource and component base classes
relevant to its own data:

+ The attribute subsystem has renamed `System` class to `Resource`, which now
  inherits `smtk::resource::Resource`.
  Also, `smtk::attribute::Attribute` inherits `smtk::resource::Component`.
+ The model subsystem has renamed `Manager` to `Resource`, which now
  inherits `smtk::resource::Resource`.
  Also, `smtk::model::Entity` inherits `smtk::resource::Component`.
+ The mesh subsystem now provides `Resource` and `Component` classes;
  mesh components refer to mesh sets.

The `PersistentObject` class requires subclasses to provide each instance
with a UUID and a name.
The `Resource` subclass adds a requirement for a location (i.e., a URL).

### Resource links

Resources and components may now be linked to each other via a role.
Roles are integers provided by SMTK (for values < 0) or
applications (for values >= 0).

As the use cases for resource/component links has been fleshing out, the
design for these links continues to refine to accommodate these cases.
SMTK weaves the use of link roles into smtk::resource::Link's
query methods, and it presents a uniform API for resource and component
links.

SMTK 3.0 introduces resource associations to `smtk::attribute::Resource`
and uses them in place of `refModelManager()`. It also changes `ModelEntityItem`
into a `ComponentItem`, facilitating its use of resource links.

## Changes to Core

### Attribute system

#### General Code Changes

+ attribute::System has been renamed to attribute::Resource
+ Added templated method smtk::attribute::Item::definitionAs<>() that will now returned the item's definition properly cast to the desired const item definition shared pointer type
+ Added associatedObjects() and disassociate() methods to smtk::Attribute to support non-model resources and resource components
+ Since attribute::ComponentItemDefinition changed its default behavior from SMTK 2.0 (it now creates a non-extensible item with a size of 1), a Definition needed to explicitly set the number of required values to 0 for its default association rule
+ Fixed bug in attribute::ReferenceItem that prevented values to be set to nullptr
+ Added an IncludeIndex property to Attribute and Definition.  This is used by I/O classes to represent include file structure
+ Added a DirectoryInfo Property to Attribute Resource.  This is used by I/O classes to represent the include file structure of the resource.
+ Added Resource::attributes(object) method to return all of the attributes associated with the persistent object that are owned by the attribute resource.
+ Added Definition::attributes(object) method to return all of the attributes associated with the persistent object that are derived from the definition or definitions derived from it.
+ Added ReferenceItem::acceptableEntries() method to return it's definition's acceptability rules.
+ Added Protected method ReferenceItemDefinition::setRole(role) method for setting the role of its related resource links.

#### Read and write operations for attribute resources

Attribute resources now have read and write operations (and associated
free functions), facilitating their generalized manipulation via
operation groups. The format for the files used in these methods is
JSON, matching the .smtk formats for models and meshes.

#### ReferenceItem, ResourceItem, and ComponentItem

The ReferenceItem (holding shared-pointer references to PersistentObject),
ResourceItem (a subclass of ReferenceItem that only allows references to Resources), and
ComponentItem (a subclass of ReferenceItem that only allows references to Components)
are new subclasses of `smtk::attribute::Item`.
Their corresponding `smtk::attribute::ItemDefinition` subclasses
hold t

#### DirectoryInfo and FileInfo

These classes were created so we can save out an attribute resource using the original include file structure.

A FileInfo instance represents the information found in an included attribute file which includes the following:

+ Catagories specified in the file
+ The set of include files that file uses
+ The default catagory that the file specified (if any)

The DirectoryInfo is simply a vector of FileInfos.  The IncludeIndex property found in Attributes, Definitions, and View corresponds to an index into this vector indicating that the instanced was read in from that file.

The first index in the array is "special" in that it represents the resource itself.  Any new Definitions, Attributes, or Views created after the resource is loaded into memory will be considered part of that "file".  This is why the default value for the IncludeIndex Property is 0!

### Changes to IO processing

+ Refactored out the processing of association information when reading in attribute::Definitions so it can be overridden
+ Implemented missing support for V1/V2 association information for attribute::Definition
+ Supporting the ability to write out resources using included files.  This allows the possibility of creating a program to update existing attribute template files to the latest version.  Currently the attributeReaderWriterTest has been modified for this very purpose.
+ Removed the ResourceSet Reader and Writer classes - in SMTK 3.0 they should no longer be needed.

#### JSON I/O

Classes, including `smtk::io::LoadJSON` and `smtk::io::SaveJSON`, that relied
on the `cJSON` library have been removed.
Their functionality is replaced by `nlohmann_json` bindings,
which are distributed throughout the codebase to reflect their role as
binding code.

The old json model file format has been replaced with a format more in
keeping with the internal structure of `smtk::model`.

### Model system

+ Classes renamed as described above.
+ Added Entity::owningModel() method to match functionality in EntityRef
+ Changed Entity::attributes() and EntityRef::attributes() to take in a const attribute::Definition shared pointer

### Mesh system

#### Introduce smtk::mesh::Component

To adhere to the Resource/Component paradigm in SMTK 3.0, we have
introduced the lightweight class smtk::mesh::Component, which
facilitates the ability to pass meshsets as component items in an
attribute.
`smtk::mesh::Component` is now used to send/retrieve meshsets to/from
attributes.

Mesh resources and components now appear in the resource tree view
and mesh component names may be edited in place using the new
SetMeshName operator.

`smtk::mesh::Manager` is now removed from SMTK.

Mesh Collections are now optionally managed by a resource manager.
This is a significant change.
Previously, mesh resources were held by a mesh manager owned by a
model resource. Now, mesh resources are freestanding and can be
classified onto a model resource. The classification mechanism is
handled internally using resource links.

#### Separate MOAB API from smtk::mesh API

One of the primary components of `smtk::mesh` is
`smtk::mesh::HandleRange`, a range-based index set used to compactly
represent large numbers of indices. It was originally a typedef of
`::moab::Range`, and its MOAB-based API had begun to creep into other
non-MOAB sections of `smtk::mesh`. This class has been replaced by
boost's `interval_set`, which has similar functionality and allows us to
more cleanly contain MOAB's backend.

`smtk::mesh::HandleRange` now has a different API that is analogous to
its original version.

### Operation system

#### Operations apply a Write lock on Resources by default

Resources and Components passed into Operations as input parameters
are now locked with Write access privelages (only one Operation can
mainpulate the resource) by default.

`smtk::operation::Operation` now has an API for other Operations to
execute the operation without first locking resources. The API uses a
variant of the PassKey pattern to only expose this functionality to
other operations. `smtk::operation::Operation` also contains a virtual
method that marks input resources with a Write LockType and resources
included in returned results as modified.

#### Add infrastructure for a centralized operation launcher

Operation managers now support instances of
`smtk::operation::Launcher`, functors designed to launch
operations. Multiple launch types are supported, and the default
launcher executes an operation on a concurrent thread.

There is also a Qt-enabled launcher that executes on a child thread
and signals on the main thread when operations are launched. The
signaling launcher will facilitate progress management in the future.

### View system

SMTK 3.0 provides a view subsystem to help with the presentation
and editing of simulation data.
It does not depend on any particular GUI libraries.

#### Phrase models

Several changes have been made to SMTK's descriptive phrase infrastructure:

+ A new subphrase generator that presents a fixed-depth tree
  is available and is the default.
+ Changes due to operations are now handled properly by the
  new subphrase generator but not by others.
+ There is a new phrase model, SelectionPhraseModel, that
  monitors a selection and populates the top-level phrases
  with selected objects.

##### Developer-facing changes

Phrase models now properly listen for and respond to operation
manager events. When an operation completes, the phrase model
should respond by updating the list of top-level phrases and
then ask the top-level subphrase generator for a list of new
phrases to insert — along with the paths at which they belong.
The top-level subphrase generator may ask other subphrase
generators to add to this list.

Deleted and modified components are handled automatically by
the base phrase model class, but created components may appear
anywhere in the tree; the work of determining where to place
these phrases must be left to other classes.

The resource panel now presents the new two-level subphrase
generator.

### Project system

New SMTK classes are added for organizing resources into simulation projects,
as outlined in a [discourse topic](https://discourse.kitware.com/t/new-modelbuilder-plugin-for-projects/176).
The primary API is provided by a new `smtk::project::Manager` class, with
methods to create, save, open, and close projects. The resources contained
by a project are represented by a new `smtk::project::Project` class.
For persistent storage, the resources contained a project are stored in a
user-specified directory on the available filesystem.

## Changes to Qt subsystem

### Expose qtReferenceItem for associations

Now qtReferenceItem (the base class for qtComponentItem and qtResourceItem)
can be instantiated and used in addition to its subclasses.
While the subclasses may choose to behave differently, it was necessary
to make qtReferenceItem available so that operation associations (which
are forced to be ReferenceItems in the attribute system so that operations
may take either resources or components) can be visualized and edited.

The qtReferenceItem (and thus its subclasses) now have improved usability:

+ they highlight their members in the 3-d view on hover;
+ they provide controls to copy their members to/from the application selection
  as well as a button to reset the item;
+ the popup for selecting members via a list is now attached to the item rather
  than appearing as an application-wide modal dialog;
+ they provide immediate visual feedback when the members are edited via
  the popup; and
+ they accept valid edits once the user clicks outside of the popup
  while allowing users to abandon edits with the escape key.

### Model Entity Centric Views

There are use cases where every model entity needs to have an attribute associated with it.
For example a simulation may require every surface to have a boundary condition associated with it.
It should be easy to glance at the presented view and determine which surfaces do and do not
have the desired association.
**qtModelEntityAttribute** is one view renderer that provides this type of functionality.
Note that each model entity will have its own attribute associated with it (no sharing). Below is a XML Example:

```xml
    <View Type="ModelEntity" Title="Surface Properties" ModelEntityFilter="f">
      <AttributeTypes>
        <Att Type="SurfaceProperty" />
      </AttributeTypes>
    </View>
```

In this case all Model Faces will be displayed and will have an attribute derived from SurfaceProperty associated with it.

### Supporting Empty Views in a Tiled Group View

If a child view is not displaying any attribute items
the owning tiled group view will no longer display the title of the child view.

### Item View Support

It is now possible to control how attribute items get presented in the Qt interface.
You can now specify an ItemView section within the Att block of the XML or JSON.  Below is a XML example:

```xml
    <View Type="Instanced" Title="Frequency Information">
      <InstancedAttributes>
        <Att Name="EigenSolver" Type="FrequencyInfo">
          <ItemViews>
            <View Item="NumEigenvalues" Type="Default" Option="SpinBox"/>
            <View Item="FrequencyShift" Option="SpinBox" StepSize="100" Decimals="0"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
```

+ The XML attribute "Item"  must match the name of the item in the SMTK attribute.  For example, NumEigenvalues is an item in a FrequencyInfo attribute.
+ The XML attribute "Type" corresponds to the Qt item class used to present the item.  If not specified it is assumed to be Default
+ The rest of the information is then processed by the Qt item class.  For example the Default qtItem class for double items is qtDoubleItem which supports the following:
    + Option="SpinBox" - use a qtDoubleSpinBox to manipulate the item.  You can specify the number of decimals and set size.
    + Option="lineEdit" (Default) - use a qtLineEdit widget to manipulate the item.
+ ItemViews are currently supported in the following views:
    + Type="Attribute" (qtAttributeView)
    + Type="Instanced" (qtInstancedView)
    + Type="ModelEntity" (qtModelEntityAttributeView)
    + Type="Operator" (qtOperatorView)

#### General Code Changes

+ qtAttributeItemWidgetFactory was removed - all functionality has been added to qtUIManager
+ All icons have been moved to icons subdirectory
+ All qtItem classes now take in a smtk::extension::AttributeItemInfo object that is used to define the instance
+ Added qtDoubleItem, qtIntItem, qtStringItem classes that use the qtInputsItem class
+ Added qtFileItem and qtDirectory classes that use the qtFileSystemItem class.
+ qtAssociationWidget has been modified to work with SMTK 3.0 version of model resources but NOT using Resource Links
    + Widget now uses model::Entity instead of model::EntityRef
+ Replaced code that was grabbing the raw pointer from a shared pointer so that this was not necessary
+ Added a IncludeIndex property to Views.  This is used by I/O classes to represent the directory structure
+ qtDescriptivePhraseModel now provides a `PhrasePtrRole` on its indexes that can be used
  to fetch a descriptive phrase shared-pointer without access to the model (only the index).
  This allows proxy models such as QSortFilterProxyModel to "wrap" descriptive phrases.

### Function evaluation feature in the SimpleExpression view

The function evaluation feature in SimpleExpression view relies on VTK.
However, also do we want to be able to preview this view with only Qt
enabled. In order to do so SMTK now creates a subclass of qtSimpleExpressionView
and it overrides the SimpleExpression view constructor at runtime.
Now users can preview it with only Qt enabled and do
function evaluation when both Qt and VTK are enabled.

## ParaView related changes

Most of the previous ModelBuilder functionality has been moved
out of the CMB repository and into SMTK as a series of libraries
and plugins that depend on ParaView.

### Introduce ParaView pipeline sources for all SMTK resources

All SMTK resources are represented in a VTK pipeline as a
vtkSMTKResource. This class holds a pointer to the SMTK resource and
can generate an appropriate converter to map the resource into a
vtkMultiBlockDataSet. The converter generation logic is extensible,
facilitating custom visualization pipelines for future resource
types.

ParaView-entwined resource access routines (file readers, file
importers, resource creation actions) derive from
vtkSMTKResourceGenerator, which derives from
vtkSMTKResource. Additionally, when a pipeline source is queried using
the associated resource as the key, a new pipeline source is now
generated in the event that a pipeline source cannot be
identified. This change facilitates the generation of ParaView
pipelines from any action that results in the creation of an SMTK
resource (in addition to the ParaView-style readers and resource
creation actions).

### ParaView representation changes

Highlighted and selected entities are no longer rendered as semi-transparent
overlays on top of the original geometry.
Instead, each visual block is rendered at most once.

Each requested render now rebuilds the list of renderable vtkDataObject
leaf-nodes in its multiblock inputs only when the input changes.
Also, the representation observes selection changes and updates a
timestamp used to determine whether block visibilities need to be
recomputed at the next render.
It is still up to the client application to force a re-render when the
selection changes.

### ParaView-widget controls for SMTK attribute items

SMTK now makes some initial widgets (ParaView's box, plane, sphere,
and spline widgets) available for controlling values of items in
an SMTK attribute.
By adding an `ItemViews` section to an attribute's view and
setting the item's `Type` attribute to the proper string (one
of `Box`, `Plane`, `Sphere`, or `Spline`), the item will be
shown using a ParaView widget (and associated 3-d editor) when
the attribute is loaded into modelbuilder or paraview.
For example:

```xml
<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="Example">
      <ItemDefinitions>
        <Double Name="bbox" NumberOfRequiredValues="6">
        </Double>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="Demo" Type="Example">
      <Items>
        <Double Name="bbox">
          <Values>
            <Val Ith="0">0.0</Val>
            <Val Ith="1">0.05</Val>
            <Val Ith="2">0.0</Val>
            <Val Ith="3">0.05</Val>
            <Val Ith="4">0.0</Val>
            <Val Ith="5">0.125</Val>
          </Values>
        </Double>
      </Items>
    </Att>
  </Attributes>
  <Views>
    <View Type="Group" Title="Sample" TopLevel="true">
      <Views>
        <View Title="Example"/>
      </Views>
    </View>
    <View Type="Instanced" Title="Example">
      <InstancedAttributes>
        <Att Type="Example" Name="Demo">
          <ItemViews>
            <View Item="bbox" Type="Box"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
```

At this time, the widget⟷item connection is one-way; the item is updated
by the widget, but changes to the item outside of editing done using the
widget does not update the widget.
The latter cannot work until operations or some other signaling mechanism
exists to inform SMTK when changes are made to attribute item-values.

A limitation of the current implementation is that changes to the active
view do not affect which view the widget resides in.

In the longer term, this functionality may be expanded to other
ParaView widgets and provide a more flexible mapping between values
in SMTK items and their representative widget values.
For example, the XML above assumes the bounding box values
are stored as a single DoubleItem with 6 required values.
However, it is also common to think of a box as specified by
2 corner points (i.e., a GroupItem with 2 DoubleItem children)
or as a center point and a vector of lengths along each axis.
By accepting more options in the XML, we will allow a mapping
between the widget and items to be specified.

Finally, note that the box widget will only initialize its representation
with values from the SMTK item if they are non-default (or if
no default exists).
Similarly, the plane widget uses a default size for its widget which
is very inflexible.
We plan to extend this capability by accepting more XML attributes
in the AttributeItemInfo's Component entry that specify how to obtain
initial values from model components currently loaded into
modelbuilder/paraview.

### Developer changes

The new classes include:

+ `pqSMTKAttributeItemWidget` — a base class for all paraview-widget items; it inherits qtItem.
+ `pqSMTKBoxItemWidget` — a subclass of pqSMTKAttributeItemWidget that realizes a box widget.
+ `pqSMTKPlaneItemWidget` — a subclass of pqSMTKAttributeItemWidget that realizes a plane widget.
+ `pqSMTKSphereItemWidget` — a subclass of pqSMTKAttributeItemWidget that realizes a sphere widget.
+ `pqSMTKSplineItemWidget` — a subclass of pqSMTKAttributeItemWidget that realizes a spline widget.

The `pqSMTKAppComponentsAutoStart` class's initializer now
calls `qtSMTKUtilities::registerItemConstructor` to register
each concrete widget class above as a `qtItem` subclass.
Once the plugin is loaded, any attributes displayed in the
attribute panel or operation panel may request, e.g., the box
widget with `Type="Box"` as shown above.

### Operation panel

SMTK now includes a paraview panel for editing model-operation
parameters (and then submitting the operation for execution).
Operations that appear are a subset of all operations;
which operations appear is determined by a whitelist of
operation names that is provided by the `OperationFilterSort`
class in a new `smtk/workflow` directory.
The subset of whitelisted operations that are available changes
in response to the active selection in the model tree view.

In more detail:

+ Add an operations panel UI.
+ Add a new smtk::view::AvailableOperators class that tracks
  a selection and an operation manager, calling observers when
  the list of operations changes (some work TBD).
+ Add `{observe,unobserve}Metadata` to smtk::operation::Manager so
  that new observers can be immediately passed the list of extant
  operation metadata.
+ Respond to selections in modelbuilder.
+ Add a `primaryAssociation()` method to operation metadata
  that returns the smtk::attribute::ReferenceItem for each
  operation (or nullptr if the operation's parameters don't
  support association).
+ Fix logic in AvailableOperators to test whether each
  operator accepts **all** the selected items and that
  the **number** of selected items is appropriate.
+ Remove some debug printouts and use the available-ops observer
  to print when the list changes.
+ Add a new subsystem to SMTK: **workflow**.
  The workflow system will host a task-dag used to provide
  suggestions to users. Currently, it exists only to
  filter+sort the list of applicable operations for
  presentation to the user.
+ Work on the AvailableOperations class in smtk/view to
  own an use the workflow operation filter.
+ Remove use of qtModelOperationWidget from custom operator views.
  This is in preparation for removal of qtModelOperationWidget,
  qtOperationDockWidget, qtOperationView, and qtModelView entirely.
  The qtModelView is replaced by the pqSMTKResourcePanel and the
  others will be replaced by the pqSMTKOperationPanel.
+ Add operation and resource managers to the qtUIManager.
  There may be a better place to put these that is not specific to Qt.
+ Test the workflow OperationFilterSort object (at least JSON
  deserialization).
+ Operation panel now lists available operations on the first selection change.
  The fact that it doesn't list an initial set of operations is a bug.
+ Fixed AvailableOperations so you can set its workflow filter.
+ Added methods to AvailableOperations to provide access to the
  user-facing data on operations that OperationFilterSort::Data provides.
+ Added the qtAvailableOperations widget. It is a subclass of QWidget
  and owns a QListWidget, which it populates with operations based on
  an AvailableOperations instance().
+ Add `createParameters` method to SpecificationOps.
+ When asked for operation parameters, always call `createParameters`
  rather than `extractParameters` so that operations always have unique
  parameters rather than sharing them.
+ Create a top-level view in the UI manager when none exists in order to
  display operators.
+ Listen for click events in the operation list widget and respond
  by showing a view of the matching operation.
+ Fix the XML reader to handle `AssociationDefs` that contain a
  `MembershipMask` tag.
+ Switch a bunch of operators to use `Accepts` entries instead of
  `MembershipMask`s in their associations (to be more specfic about what
  types of resources they accept).
+ Instantiate the pqPluginSMTKViewBehavior class inside
  pqSMTKAppComponentsAutoStart. Otherwise, custom views
  are not registered to the Qt UI manager.
+ The "name" of the assign colors operation has changed
  from "assign colors" to "smtk::model::AssignColors".
  Partially fix the custom view so that the UI is created.
+ Fix misspelled method.
+ Fix polygon "split edge" operation (only 1 edge can be split at a time).
+ Remove debug cruft.
+ Improve operator availability logic.
+ Refactor qtUIManager and pqSMTKOperationPanel.
  Now a qtUIManager may be constructed with an Operation instance
  whose attribute resource will be rendered as its view.
  Furthermore, applications can ask the qtUIManager to find/create a
  default view of its operation.
  This greatly simplifies the operation panel's job.
+ Rename `displayOperation()` methods in the operation panel to
  `editOperation()`, which is more accurate.
+ Fix `qtUIManager::initializeUI()` to create the proper type of
  view info when constructed with an operation.
+ Have the operation panel associate the current selection to the
  chosen operation when double-clicking.
+ Add an option to show **all** operations (or only those
  that match the current selection) to the AvailableOperations
  object. Also, expose it in the operation panel.
+ Have the operation view actually execute the operation,
  not just signal that it wants the operation run.
+ Make the polygon Write operation use associations
  for the resource it will write rather than an Item.
+ Make `smtk::view::Selection` hold `smtk::resource::PersistentObject::Ptr`s
  instead of component pointers. This will allow the operation
  panel to expose operations such as polygon::Write.
+ Add some example "workflow" files to the data directory.

### Cumulus Jobs Panel

We have added a dock widget for monitoring and interacting with jobs
submitted by Cumulus. Currently, the API is tailored to NERSC. As
additional submission sites are added to our workflows, the API will
likely be made more general.

### Enforce ParaView semantics for apply() on File→Open only

When resoures are loaded via File→Open, ParaView semantics
apply: the apply button must be pressed to complete opening the
file. This matches the functionality of the other ParaView readers
(that are selectable when there are multiple readers for a file type).

When a resource is created via File→New Resource, a modal dialog
is presented to the user. When the user presses apply on the dialog,
the resource is generated and its representation is rendered. The
Properties Panel's apply button is never enabled.

When a resource is created by an operation from the Operation
Panel, the resource is generated and its representation is
rendered. The Properties Panel's apply button is never enabled.

### Introduce ParaView behavior that adds New Resource

Operations that share a common theme in the contexts of functionality
and graphical presentation are added to common operation groups. We
have constructed a new operation group for the creation of new
Resources. To utilize this group, we have also  added a file menu
option that allows a user to create a new Resource.

When there are multiple create operators registered to a single
Resource, the Resource action is listed as a menu that contains a list
of the individual create operations.

ParaView applications that load SMTK's pqAppComponents plugin now have
access to a menu action to construct a new SMTK resource, if an
operation is associated to that resource in the operation CreatorGroup.

### Introduce ParaView behavior that imports files into an existing SMTK resource

We have added a file menu option that allows a user to select a data
file and import its contents into an existing resource. This differs
from the canonical File→Open method, which creates a new resource for
the imported data.

ParaView applications that load SMTK's pqAppComponents plugin now have
access to a menu action to import a data file into an existing resource.

### Introduce ParaView behavior that exports smtk simulations

We have added a file menu option that allows a user to select a python
file describing an SMTK operation for exporting simulations. Once
selected, the operation's parameters are displayed in a modal
dialog. Upon execution, the operation is removed from the operation
manager.

ParaView applications that load SMTK's pqAppComponents plugin now have
access to a menu action to export an SMTK simulation.

### Introduce ParaView behavior that adds Save Resource (As...)

Our goal is to have ModelBuilder be as minimially modified from stock
ParaView as possible. To this end, we have added a behavior to SMTK's
pqAppComponents plugin that inserts "Save Resource" and "Save Resource
As..." menu actions to the File menu.
The icon currently used for saving a resource is identical to
ParaView's icon for saving data. In the future we may want a unique
icon for reading/writing SMTK native resources.
These changes are an alternative to the changes in model builder
described
[here](https://gitlab.kitware.com/cmb/cmb/merge_requests/612); the
concerns identified therein are still applicable.

ParaView applications that load SMTK's pqAppComponents plugin now have
access to menu actions to save resources into SMTK's native (`*.smtk`) format.

### Importing SMTK python operations

We have added a file menu option that allows a user to select a python
file describing an SMTK operation and import this operation into a
server's operation manager.

ParaView applications that load SMTK's pqAppComponents plugin now have
access to a menu action to import a python-based SMTK operation.

### ParaView Pipeline-Selection Synchronization

There is now a `pqSMTKPipelineSelectionBehavior` class that,
when instantiated by a ParaView plugin, updates the SMTK
selection when an SMTK resource is activated in the PV pipeline
browser and vice versa when an SMTK resource is selected and
has a pipeline-browser entry.

Previously, this functionality was provided by the
pqSMTKResourcePanel but is now split into a separate behavior.

### Redirect singleton smtk::io::Logger to output to ParaView output window

The singleton smtk::io::Logger instance now pipes its records to Qt's
messaging system, which is intercepted by ParaView and displayed in
ParaView's output window.

ParaView applications that load SMTK's pqAppComponents plugin now can
read any SMTK messages using ParaView's output window.

### ParaView Settings

SMTK now uses ParaView's user preferences dialog.

#### User-visible changes

Settings for SMTK (and specifically, a setting to highlight
objects in 3D render-views when the mouse is hovered over the
matching item in a list/tree view) now appear in ParaView's
user-preference settings dialog.

#### Developer-facing changes

Going forward, SMTK should use ParaView's pattern for exposing
user-adjustable settings:

+ a server-side XML entry should exist in `smtk/extension/paraview/server/smconfig.xml` or
  another, similar XML config file. The XML entry must be in the "settings" ProxyGroup
  and also have an entry in a PropertyGroup that exposes it.
+ the property should be a member variable managed by a singleton in the
  same way that `vtkSMTKSettings::HighlightOnHover` is managed.
  If appropriate, place the member variable in vtkSMTKSettings;
  otherwise, create a new singleton.
+ any classes that wish to use the setting should then obtain the singleton
  holding the setting, fetch its value, and
    + immediately use the value (re-fetching each time just before it is used) or
    + observe changes to the setting via the Qt-VTK signal interop layer, e.g.,

```c++
pqCoreUtilities::connect(
  vtkSMTKSettings::GetInstance(), vtkCommand::ModifiedEvent,
  qtObjectPointer, SLOT(methodThatRespondsToSettingChange()));
```

## Python system

### Support for python 3

We have added support for building and testing smtk's python bindings
for use with python 2 and 3. Python tests and examples included within
SMTK's source should hereafter be both Python 2 and Python 3-compliant.

### Conda support

SMTK 3.0 adds a recipe for building a "lite" version of SMTK as a conda package.

## Build system and documentation

### Documentation options

Now instead of the `SMTK_ENABLE_DOCUMENTATION` CMake option,
SMTK provides `SMTK_BUILD_DOCUMENTATION`,
which is an enumerated string option that can be

+ `never` — No documentation is built, and no documentation tools are required.
+ `manual` — Only build when requested; documentation tools (doxygen and
  sphinx) must be located during configuration.
+ `always` — Build documentation as part of the default target; documentation
  tools are required. This is useful for automated builds that
  need "make; make install" to work, since installation will fail
  if no documentation is built.

Our build infrastructure uses `always` but most developers will prefer `manual`.

### Support for SMTK as a third party submodule

SMTK 3.0 replaces `CMAKE_PROJECT_NAME` with `PROJECT_NAME` within SMTK so
SMTK will build and install as a third party submodule of CMB. Also,
the "subproject" delineation was removed from cJSON (which will soon
be removed entirely), SMTKVTKExtensionMeshing, and
SMTKDiscreteModel. Finally, the header test macro has been modified to
accept as input the library associated with the header files.

### Support for SMTK as a development SDK

Several build-related updates have been made to facilitate the use of
SMTK as a development SDK, so SMTK plugins can be developed using the
installed version of SMTK.

## Sessions

The bridge directory and namespace has been renamed to session to
better conceptualize the idea of representing the native modeling
session used by a model resource.

### RGG session

The RGG session, originally added to SMTK by Haocheng Liu, is a
modeling session for designing reactor cores. This session has been
updated to conform to the latest API in SMTK.

### VTK session

The Exodus session has been renamed to the VTK session as a
more apropos name given the types of supported files.
