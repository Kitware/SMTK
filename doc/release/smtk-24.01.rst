.. _release-notes-24.01:

=========================
SMTK 24.01 Release Notes
=========================

See also :ref:`release-notes-23.04` for previous changes.

API-Breaking Changes
====================

We have been forced to remove ``smtk::task::Adaptor::reconfigureTask()`` which has been replaced by ``smtk::task::Adaptor::updateDownstreamTask(State upstreamPrev, State upstreamNext)`` and is a pure virtual method that must be overridden.  Normally we would have deprecated the method but the issue is that derived classes may have overridden reconfigureTask and it would have been difficult to indicate these methods needed to be replaced.  By removing this virtual method, the compiler will now see these overridden methods as errors.

SMTK Common Related Changes
=====================================

Default observer priority
-------------------------

The :smtk:`smtk::common::Observers` template now uses a default priority
of 0 instead of ``std::numeric_limits<int>::lowest()`` (which is a negative number).
Any code that inserts observers using the signature that does not take an explicit
priority may now invoke that observer earlier than in previous releases of SMTK.
If you wish to maintain the old behavior, you must now explicitly pass a priority.

The :smtk:`smtk::common::Observers` template now provides methods
named ``defaultPriority()`` and ``lowestPriority()`` for your convenience.

This change was made to facilitate observers provided by SMTK that need to ensure
they are the last invoked after an operation since they release objects from managers
and may invalidate the operation result object.

TypeContainer Changes
---------------------

A few minor changes were made to :smtk:`smtk::common::TypeContainer`:

+ Methods and variables that were previously private are now protected so that
  this class can be subclassed.
+ The wrapper class used to store objects in the type container now provides a
  string token holding the type-name of the inserted type.
  This is used by the new :smtk:`smtk::common::RuntimeTypeContainer` subclass described below.
+ The ``insert_or_assign()`` method has been renamed ``insertOrAssign()``
  to be consistent with the rest of SMTK.
  The original name is deprecated and will be removed in a future version of SMTK.

New RuntimeTypeContainer
------------------------

:smtk:`smtk::common::RuntimeTypeContainer` is a new subclass of TypeContainer.
The base TypeContainer class can only hold a single object of a given type.
When applications handle many objects that share a common base type (i.e., whose
complete type is unknown since only a pointer to a base type is held),
there was no way to insert these distinct objects into a TypeContainer even if
their complete types were distinct.

To resolve this issue, the RuntimeTypeContainer class allows you to insert
objects by their base type but use a "declared type-name" as the key.
As long as these declared type-names are unique, multiple objects sharing the
base type can be held by the container simultaneously.

See the class and its test for detailed documentation.

Type Hierarchy Reflection
-------------------------

SMTK has long provided ``smtkTypeMacro()`` and ``smtkSuperclassMacro()``
in order to provide classes with type-aliases and virtual methods that
allow reflection of an object's type.
This has now been extended to provide (when a ``Superclass`` type-alias
is present) the entire inheritance hierarchy.

In addition to the virtual ``typeName()`` method, each class that uses
``smtkTypeMacro()`` or ``smtkTypeMacroBase()`` now provides

* ``matchesType(smtk::string::Token baseType)`` – which returns true
  if the object is or inherits the given base type
* ``classHierarchy()`` – which returns a vector of string-tokens
  holding the type-names of the object and its base classes.
* ``generationsFromBase(smtk::string::Token baseType)`` – which returns
  an integer indicating the number of "hops" along the inheritance tree
  required to get from the object's type to the given base type.

See ``smtk/common/testing/cxx/UnitTestTypeHierarchy.cxx`` for examples.

SMTK Resource Related Changes
=============================

Resource Manager No Longer Adds Created Resources To Itself
-----------------------------------------------------------

While some signatures of ``smtk::resource::Manager::create()``
no longer added their returned resources to the resource
manager, not all of them did.
This inconsistency has been corrected such that no call to
create a resource adds it to the manager; this required
changes to several tests.
If your project relies on resources being automatically
added to the resource manager as it creates them, you will need
to change your code manually add them with ``smtk::resource::Manager::add()``
after your call to ``create()``.

Note that in proper applications (as opposed to tests or batch
scripts), you should always create resources inside an
:smtk:`Operation <smtk::operation::Operation>` and append it
to a :smtk:`smtk::attribute::ResourceItem` named "resource"
in your operation's result. This will result in the resource
being added to the manager at the completion of the operation
rather than immediately (during the operation).
The immediate addition of newly-created (and thus empty) resources
was problematic when the resource was further modified by the
operation since the the order of observations in Qt-based applications
cause the application to ignore newly-created components in the
new resource.

Removed the MODIFIED Event from the Resource Manager
----------------------------------------------------

It was determined that this event type was redundant since actions that would cause a Resource to be modified should be done via operations which would produce they own events.

In addition, it was observed that in some cases, operations that would change a Resource's **clean** state, would trigger the Resource's manager to emit its MODIFIED event which caused observer issues.

**Note:** The Resource::setClean method was the only thing that would explicitly cause the MODIFIED event to be emitted (though other methods do call setClean) and no longer does so.

Supporting Object Type Labels in the Resource Manager
-----------------------------------------------------

The :smtk:`smtk::resource::Manager` class now provides an ``objectTypeLabels()``
method returning a map that registrars can use to register human-readable
(and application-specific) strings given an object's type-name.
This method is intended to map fully-qualified type-names for *any* class to
labels that are descriptive to users, not just subclasses of
:smtk:`smtk::resource::PersistentObject`.
Labels should be as short as possible while remaining descriptive;
applications should not expect labels to be sentences or paragraphs of text.

If you use the ``smtkTypeMacroBase()``/``smtkTypeMacro()`` macros,
you can use the virtual ``typeToken()`` method on any object to identify its
class name and search the map returned by ``objectTypeLabels()`` to obtain
a human-readable string.

You can also use the :smtk:`smtk::common::typeName` template to identify a
string for any class and look it up in the map.

Applications should only look names up; registrars should write data to the map.
If existing registrars use a name that is unsuitable for your application,
simply create a registrar in your application whose ``Dependencies`` tuple
lists these registrars and overwrite their strings with ones better for your
application; because your application registrar depends on others, it will
always be invoked last.

Currently, this facility is only used by the diagram panel. (In the future,
descriptive phrases may also adopt these type labels.)

SMTK Attribute Related Changes
==============================

Expanding Units Support in Attribute Resource
---------------------------------------------

SMTK's Attribute DoubleItems and DoubleItemDefinitions can now support units specified as Defaults
as well as values.  When a default's or value's units differ from those defined by the Item's Definition,
they are converted into the Definition's units.  If no conversion is possible then the method assigning
the default or value will fail.

The Item's value(...) methods will always return the value in the units specified in its Definition.
The Item's valueAsString(...) methods will always return a string based on the unconverted value

When specifying Default Values (for DoubleItemDefinitions) and Values (for DoubleItems) the following is the expected behavior:

``DoubleItemDefinition::setDefaultValueAsString`` and ``DoubleItemDefinition::setDefaultValue``

1. Will only append units to its default string values iff its units are supported by its units system
2. Will remove the units from a default value string iff its units are not supported by its units system
3. Its static splitStringStartingDouble method now trims both the value and units strings it returns.
4. Added a hasSupportedUnits method that returns true if the Definition's units are set and are supported by its units system.

``DoubleItem::setValue and DoubleItem::setValueFromString``

1. Will only append units if its definition's units are supported by its units system and the input value string does not contains units
2. Will remove units from an input value string if its definition's units are not supported by its units system

Developer changes
~~~~~~~~~~~~~~~~~~

SMTK Resources can now hold a units::System.  In the case of an Attribute Resource, it will have a
default units::System associated with it at construction time; however, this can be replaced as long
as there are no Definitions defined within the Resource.

New Resource Methods:

* ``setUnitsSystem(const shared_ptr<units::System> & unitsSystem)``
* ``const shared_ptr<units::System> & unitsSystem() const;``

All ItemDefintions now hold onto a units::System.  The methods are protected and are identical to the ones added to Resource.

DoubleItemDefinition has the following new methods:

* ``bool setDefaultValue(const double& val, const std::string& units);``
* ``bool setDefaultValue(const std::vector<double>& vals, const std::string& units);``
* ``bool setDefaultValueAsString(const std::string& val);``
* ``bool setDefaultValueAsString(std::size_t element, const std::string& val);``
* ``bool setDefaultValueAsString(const std::vector<std::string>& vals);``
* ``const std::string defaultValueAsString(std::size_t element = 0) const;``
* ``const std::vector<std::string> defaultValuesAsStrings() const;``
* ``bool hasSupportedUnits() const;``

DoubleItem has the following new methods:

* ``bool setValue(std::size_t element, const double& val, const std::string& units);``

In addition, `DoubleItem::setValueFromString` method can now handle strings that include a double
followed by an option units.  For example "20 m/s".

Supporting Templates in Attribute XML Files
-------------------------------------------

Templates are a new feature for SMTK's XML-based attribute file format (.sbt, .sbi extensions) version 7 and later.  Templates are an extension to the existing ItemBlock concept.  The main difference between an ItemBlock and a Template is that a Template's contents can be parameterized.  When a Template is instantiated, these parameters can be assigned different values and will thereby change the information being copied.  A Template's parameter can also be given a default value.

**Note**  All parameters that do not have a default value must be given values when the Template is instanced.

Here is an example:

.. code-block:: xml

  <Templates>
    <Template Name="SimpleStingDefault">
      <Parameters>
        <Param Name="a">dog</Param>
      </Parameters>
      <Contents>
        <DefaultValue>{a}</DefaultValue>
      </Contents>
    </Template>
  </Templates>
  <Definitions>
    <AttDef Type="A">
      <ItemDefinitions>
        <String Name="s1">
          <Template Name="SimpleStingDefault">
            <Param Name="a">cat</Param>
          </Template>
        </String>
        <String Name="s2">
          <Template Name="SimpleStingDefault"/>
        </String>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

See data/attribute/attribute_collection/TemplateTest.sbt and smtk/attribute/testing/cxx/unitTemplates.cxx for examples.  You can also read the discourse on the topic here: https://discourse.kitware.com/t/adding-parameterized-blocks-to-sbt-files/1013/4.

Item Assignment Return Type has Changed
---------------------------------------

Previously, when you called ``smtk::attribute::Item::assign()``, it would return
a boolean value indicating success or failure.
However, in the case of a successful assignment, there was no way to determine if
the target item and/or its children were actually modified.
Now, this method returns an object, :smtk:`smtk::common::Status`,
that may be queried for both ``success()`` and ``modified()``.

This change is needed so that task adaptors (and others) can determine whether
to mark :smtk:`smtk::task::SubmitOperation` tasks as needing to be re-run.

**Note** The new return type provides a bool operator so that it is compatible with the previous API.

Added Ability to Ignore Categories w/r Definitions and Attributes
-----------------------------------------------------------------

You can now indicate that an Attribute's or Definition's validity and/or relevance does not depend on the Resource's active categories.  This can be very useful in the case of Definitions and Attributes that model Analyses since they tend to control the set of Active Categories and therefore do not depend on them.

The new methods are:

* Definition::ignoreCategories() const;
* Definition::setIgnoreCategories(bool val);

This information can be specified in SBT files as a XML Attribute called IgnoreCategories and is saved in the Attribute Resource's JSON and XML format.  Python bindings have also been added.

Referencing Associations via ``itemAtPath()``
----------------------------------------------

You can now reference an attribute's associations with the
:smtk:`itemAtPath() <smtk::attribute::Attribute::itemAtPath>`
method by passing in the name assigned to the association-definition rule.
To support this, :smtk:`smtk::attribute::Definition::createLocalAssociationRule`
now accepts a name string. You may also specify the association-rule's name
via XML.

**Note:** that if an item with the same name as the association rule exists, that
item will always be returned instead of the association rule.
You are responsible for ensuring names are unique; SMTK will not prevent
name collisions with the association rule.

Accessing ReferenceItem Iterator Values
---------------------------------------

The ``ReferenceItem::const_iterator`` class now provides
a templated ``as()`` method that will dynamically cast
shared pointers to a child class. Note that this method
will return null shared pointers when the object's type
is mismatched.

ValueItem Validity now include Enum Applicability
-------------------------------------------------

isValid will now take into consideration if the enum it is set to is applicable with respects to the resource's set of active categories.

See attribute/testing/cxx/unitCategories.cxx for an example.

Attribute Import Operation Changes
----------------------------------

Since Import's internal AttributeReader uses a Logger instance to keep track of errors encountered while doing its parsing, using the system Logger can be problematic.  The main issue is that several operations can be running at the same time and if any of them adds Errors to the Logger while Import is running, it can result in the Import operation thinking that it failed (or in the previous errors getting erased since the Attribute Reader resets the logger when it starts).

To fix this, the Import operation now passes in local Logger into the AttributeReader which is then merged with the system Logger after the Import is completed.

Signal Operation Change
-----------------------

The :smtk:`Signal <smtk::attribute::Signal>` operation now has a new
resource-item named `resourcesCreated` which can be used to indicate
new attribute resources that were created externally and should be
added to the application's resource manager (by the base operation
class once observers have fired).

As an example, this feature is used by the `unitBadge` test to
indicate that its programmatically-created attribute resource
should be added to the phrase model used to exercise the badge
system.

SMTK Operation Related Changes
==============================

Operations can customize what resources they lock
-------------------------------------------------

The base :smtk:`Operation <smtk::operation::Operation>` class now provides
a virtual method, ``identifyLocksRequired()`` to allow subclasses to customize
the default set of resources to be locked and their lock levels (read vs. write).
This allows operations that may need to lock components related (say by
:smtk:`Links <smtk::resource::Links>`) to external resources to include those
resources in the operation's lock set.

Export Selected Faceset as an STL, OBJ, or a PLY file
-----------------------------------------------------

A new operation (:smtk:`smtk::geometry::ExportFaceset`) has been added that
extracts the faceset of an associated component in a resource and exports
the same to a specified STL / OBJ / PLY file.

In the user-interface, this operation has a custom view that shows a tree
view of the loaded resources, from which a single component can be selected
for export.

Added GroupOps
--------------
A new ``smtk/operation/GroupOps.h`` header has been added that provides
methods to apply an :smtk:`operation group <smtk::operation::Group>` to
a container of :smtk::`smtk::resource::PersistentObject` instances.
The function will identify the proper set of operations which associate
to each persistent object in the container and, if all are able to operate,
will launch these operations with their associated objects.

In the event some objects cannot associate to any operation in the group
or some operations are unable to operate as created, a lambda provided to
the function is invoked to obtain feedback from users before launching or
aborting.

A specific function is provided to invoke the above for the :smtk:`smtk::operation::DeleterGroup`
along with a Qt-based function, :smtk:`smtk::extension::qtDeleterDisposition`
that can be passed as the final argument for querying users about deleting
objects with dependencies.

This refactors and improves code from :smtk:`smtk::extension::qtResourceBrowser`,
which would only launch a single operation from the deleter-group (requiring it to
associate to every persistent object provided).

Removing Resources
------------------

The :smtk:`smtk::operation::RemoveResource` operation now accepts multiple
resources to remove as a convenient alternative to repeatedly running the
operation with a different resource each time. (The operation was programmed
to do this but a bug in its allowed associations prevented it from being
passed multiple inputs.)

SMTK Changes to Graph Resource
==============================

Run-time Arcs
-------------

Graph resources now support the run-time creation of arc types.
This is implemented by making the :smtk:`smtk::graph::ArcImplementation` template
inherit and implement a pure virtual :smtk:`smtk::graph::ArcImplementationBase` class.
Arc types may be created via a new :smtk:`smtk::graph::CreateArcType` operation
and arcs of any type may be created via the :smtk:`smtk::graph::CreateArc` operation.

See the :ref:`smtk-qt-sys` documentation for user-interface elements that support
arc creation and deletion.

Graph arc storage
~~~~~~~~~~~~~~~~~

In order to support run-time arc types, the :smtk:`smtk::graph::ArcMap` class
no longer inherits :smtk:`smtk::common::TypeContainer`.
Instead, it owns an unordered map from string tokens to shared pointers to
:smtk:`smtk::graph::ArcImplementationBase`.
The :smtk:`smtk::graph::ArcImplementation` template inherits this base type
in order to provide virtual-method access to arcs (in addition to the high
speed interface unique to the arc traits object).

Finding Corresponding Nodes Along an Arc
----------------------------------------

The graph resource now includes a function, ``findArcCorrespondences()``,
that takes in an arc type (as a template parameter),
a pair of nodes (say ``n1`` and ``n2``),
and a lambda that can compare nodes via the arc
(one to ``n1`` and the other attached to ``n2``).
The function then returns pairs of nodes attached to ``n1`` and ``n2``,
respectively.
If no correspondence for a node is found, then a null pointer is
stored in one entry of the pair.

Query Grammar
-------------

The graph-resource :smtk:`query grammar <smtk::graph::filter::Grammar>` has
been extended to allow "bare" component type-names.

For example, if your filter-query was ``'SomeNodeType' [string{'name'='foo'}]``,
it is now also legal to write ``SomeNodeType [string{'name'='foo'}]`` (i.e., no
single-quotes required around the node's type-name).
This simplifies some upcoming changes for run-time arcs.
Single-quoted component names imply an exact match to the object type,
while bare type-names also match derived objects.
For example, if class ``B`` inherits ``A``, then a filter-query ``'A'`` will only
match instances of ``A`` and not ``B`` while ``A`` will match instances
of ``B`` as well.

This testing of derived types is accomplished
by checking whether a query-filter token is present in
a `std::unordered_set<smtk::string::Token>` computed once at run-time, so
it is efficient.

SMTK Project Related Changes
============================

Filtering Changes
-----------------
Now :smtk:`smtk::project::Project`'s ``queryOperation()`` method
supports component type-name and property queries.
This can be used to fetch tasks and worklets by type.
Note that the inheritance hierarchy of components is also available
to the query system, so you can, for example,
expect a query on ``smtk::task::Task`` to return objects of
type ``smtk::task::FillOutAttributes``.

Project Manager Changes
-----------------------

Project management at creation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The project manager no longer automatically manages projects as they are created.

Instead, the project manager observes operations which create projects and manage
any new projects upon completion. This matches the pattern set by the resource
manager and avoids observers being fired during operations when the project may
not be in a valid state.

If your code explicitly calls ``smtk::project::Manager::create(typeName)`` outside
of an operation, you now need to explicitly ``add()`` the project to the manager.
If you call ``create(typeName)`` inside an operation, you must be sure to add the
project to your operation's Result (in a ReferenceItem) so it can be added.
If you call ``smtk::project::Manager::remove(project)`` inside an operation, you
should not do so any longer. Instead, you should add the project to the
operation-result's ``resourcesToExpunge`` ReferenceItem. The base Operation class
will remove the project from its manager after the operation's observers have been
invoked to properly order Operation and Resource observers before Project observers.

Project addition
~~~~~~~~~~~~~~~~

The project manager's observer was being invoked twice each time a project was
added because multiple calls to ``smtk::project::Manager::add()`` were made and
no checking was done to prevent the second call from succeeding even though the
project was already managed. This has been fixed.

Project removal
~~~~~~~~~~~~~~~

The project manager's observers were not informed when a project was removed
from the manager; now they are.

Changing Project I/O Operations
-----------------------------

Previously reading in the resources of a project used the read mechanism registered with the Resource Manager; however, resources loaded in this way may be processed differently than creating a reader via the operation manager. This happens because the operation manager typically has application-specific data in its ``smtk::common::Managers`` object and it provides this to operations it creates. In the case of attribute resources loaded by the resource manager, evaluators were not properly set while using the ReadResource approach did not have this issue.

These changes allow the ResourceContainer deserialization function to internally call the ReadResource Operation through the use of a operation::Helper which provides an operation::Operation::Key.

SMTK Task Related Changes
================================

Tasks are now Components
------------------------

The :smtk:`smtk::task::Task` class now inherits :smtk:`smtk::resource::Component`.
Instances of tasks are still owned by the task manager, but it is now assumed that
the task manager is owned by a :smtk:`Resource <smtk::resource::Resource>`.
This has far-reaching consequences:

+ Tasks may have properties and links (such as associations to attributes).
+ Tasks must not be modified outside of operations (for thread-safety).
+ The parent resource must provide operations to find, insert, and remove tasks.
  The Project class now provides these.

Adaptors are now Components
---------------------------

The :smtk:`smtk::task::Adaptor` class now inherits :smtk:`smtk::resource::Component`.
Instances of adaptors are still owned by the task manager, but it is now assumed that
the task manager is owned by a :smtk:`Resource <smtk::resource::Resource>`.
This has far-reaching consequences:

+ Adaptors may have properties and links (such as associations to attributes).
  Note that links are **not** used to model connections from the adaptor to
  its upstream and downstream task.
+ Adaptors must not be modified outside of operations (for thread-safety).
+ The parent resource must provide methods/operations to find, insert, and remove adaptors.
  The Project class now provide methods to find and filter adaptors;
  it provides a read operator that may create adaptors.
  The task system also provides an operator (:smtk:`smtk::task::EmplaceWorklet`)
  that may create adaptors.

Introducing Worklets and Galleries to the Task Manager
------------------------------------------------------

There are times when a user will need to interactively extend a task workflow  by adding a tasks or a group of related tasks.  To provide this functionality, SMTK provide the concept of a :smtk:smtk::task::Worklet.  A worklet is defined as an object representing a template for a set of tasks that can be instantiated to reuse some portion of a workflow. In SMTK, a worklet is a subclass of :smtk:smtk::resource::Component.

To manager the worklets, a Gallery class has been added called smtk::task::Gallery and is held by a project's :smtk:smtk::task::Manager.

Developer changes
~~~~~~~~~~~~~~~~~~

* Added the Worklet class and all related JSON and Pybind support
* Added the Gallery class and its Pybind support (it does not need any special JSON support)
* Extended Task Manager to have the following methods:
  * smtk::task::Gallery& gallery()  - to return a gallery of worklets
  * const smtk::task::Gallery& gallery() const - to return a const gallery of worklets

New Task and Adaptor Types
--------------------------

A Task for Submitting Operations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There is a new :smtk:`smtk::task::SubmitOperation` class for
situations where users must prepare an operation to be run
(or should run an operation iteratively until satisfied).
See `task-submit-operation`_ for more information.

The operation parameter-editor panel responds to this new
task by displaying the operation parameters corresponding
to the task.
See `smtk-pv-parameter-editor-panel`_ for more information.


An Adaptor for Configuring SubmitOperation Tasks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There is a new :smtk:``smtk::task::adaptor::ConfigureOperation``
class for situations where parts of an SMTK operation
managed by a :smtk:``smtk::task::SubmitOperation`` are
configured by an upstream :smtk:``smtk::task::FillOutAttributes``
task.


Changes to task::Adaptor API
----------------------------

The signature for ``task::Adaptor::reconfigureTask()`` has changed.
Adaptors must now provide a method that takes two arguments:
the state of the upstream task before and after the current event.
This information is now passed so that adaptors can fire on any
transition between task states, not just from non-completable to completable.

If you have written a custom adaptor, you will need to change its
signature. An easy way to maintain your adaptor's current behavior
with the new API is to change your class (Foo) from something like this:

.. code:: cpp

   class Foo : public smtk::task::Adaptor
   {
   public:
     // …
     bool reconfigureTask() override
     {
       bool didChange = false;
       // Update this->to(),setting didChange on modification.
       return didChange;
     }
   };

to the following (by adding arguments and a small early-exit conditional
to your existing implementation):

.. code:: cpp

   class Foo : public smtk::task::Adaptor
   {
   public:
     // …
     bool reconfigureTask(smtk::task::State prev, smtk::task::State next) override
     {
       if (
         next < smtk::task::State::Completed ||
         prev > smtk::task::State::Completable)
       {
         return false;
       }
       // Update this->to().
       return didChange;
     }
   };

All Task Observers have Changed
-------------------------------

The task-instance, adaptor-instance, and workflow-event observers have all
changed to accommodate the fact that tasks and adaptors now inherit
:smtk:`smtk::resource::Component` and thus should only be modified inside
operations.

Because tasks and adaptors are modified inside operations (which run on
threads, not in the GUI thread), any observers to task/adaptor/workflow
events (creation/destruction/modification) must not be invoked immediately.

+ Instead of using the observers provided by :smtk:`smtk::task::Instances`
  and :smtk:`smtk::task::adaptor::Instances`, use the observers returned
  by the task-manager's ``taskObservers()`` and ``adaptorObservers()``
  methods, respectively.
+ Instead of using the workflow observer formerly provided by
  :smtk:`smtk::task::Instances` (which has been removed), use the
  task-manager's ``workflowObservers()`` method.

All of the observers provided by the task manager are initiated
by observing operations; the task-manager observes the operation manager
with a priority of ``operationObserverPriority()`` and invokes the observers
above as needed after each operation.

All of the Qt thread-forwarding for the previous observers has been
removed since operation observers (and thus the task-manager's task-related
observers) already run on the GUI thread.

Parent resource method
----------------------

The `smtk::task::Manager::resource()` method has changed to return
a raw pointer. This is because we cannot construct a weak pointer
inside the constructor of any resource/project that will own a
task manager. Since it cannot be initialized at construction of its
parent, we have switched to holding a raw pointer.

Strict Dependency Processing
----------------------------

The base :smtk:`smtk::task::Task` class now has an additional configuration
parameter indicating whether its dependencies should be strictly enforced
when computing its state or not.
You can call the ``areDependenciesStrict()`` method to see if it is set
or not (the default is false).

There is not currently a method to change whether dependencies are strictly
enforced or not since it is not intended to be changed during a workflow
but rather set at the time the workflow is designed. This may change but
would require significant work.

When strict dependency checking is enabled, its state will be Unavailable
until all of its dependencies are marked Completed by a user (not just
when they are Completable).

Removed Task Related API
------------------------
The deprecated task-constructor signatures (that did not require
a reference to a task manager) have been removed. If your application
was using these methods, you must switch to versions that pass a
task manager.

Task Related Bug Fixes
----------------------

Task::updateState
~~~~~~~~~~~~~~~~~

This method was not factoring in the task's internal state when determining either the task's current or new state which resulted in the graphical representation of the task node being incorrect.

SMTK View Related Changes
=========================

Added the Ability to Hide an Attribute's Item In a View
-------------------------------------------------------

SMTK's AttributeItemViews can now support *null* type.  By setting **Type="null"**
in an Attribute Item View, that specific item (and of its related children) will not be
displayed in a View.

Please look at data/attribute/attribute_collection/NullItemViewExample.sbt for an
example that uses this capability.

Developer changes
~~~~~~~~~~~~~~~~~~

In order to cleanly support this, qtAttributeItemInfo now provides a *toBeDisplayed* method that will
return true iff all of the following conditions are met:

* The instance has a valid smtk::attribute::item
* There is either no baseView or the baseView indicates that the item should be displayed
* There is either no ItemView Configuration or that its type is not set to *null*

Added Ability to Exclude Associations in Instance Views
-------------------------------------------------------

You can now prevent an Attribute's Associations from being displayed in an Instance View using **ExcludeAssocations**.
Note that this is not necessary if the Attribute's Definition does not have Associations specified

.. code-block:: xml

  <Views>
    <View Type="Instanced" Title="General">
      <InstancedAttributes>
        <Att Name="numerics-att" Type="numerics" ExcludeAssocations="true"/>
      </InstancedAttributes>
    </View>
  </Views>

Adding the Concept of UI Element State
--------------------------------------

SMTK provides :smtk:`smtk::view::Configuration` so that XML-formatted documents
(such as attribute XML files) can specify views for particular workflow tasks.
There are also times when application-provided user interface (UI) elements
(such as a panel or editor) needs to be configured at runtime with document-specific
JSON data.
For example when loading in a :smtk:`project <smtk::project::Project>`,
which contains a workflow of tasks that have been previously laid out in a :smtk:`diagram <smtk::extension::qtDiagram>`,
the diagram will need to be given this information when visually reconstructing the original graph.
Since this information is not part of the task, it should not be stored with the task JSON information.

To address this issue, SMTK has added a :smtk:`UIElementState <smtk::view::UIElementState>` class
that conceptually provides an API to configure the UI element's state represented as JSON and
to retrieve its current configuration as a JSON representation.

The UIElementState class is intended to be subclassed to produce and consume state data that is
relevant to a specific element in the application's user interface (e.g., panel, menu, etc.).

SMTK's :smtk:`view manager <smtk::view::Manager>` now holds a map from application UI element names
(provided by the application) to instances of classes derived from UIElementState.
Application UI elements should own an instances of UIElementState specific to itself
and insert it into (or remove it from) the view manager when the UI element is constructed
For example, an operation reading/writing a project may wish to read/write configuration of
the user interface by iterating over the view manager's map.

The first UI element to implement element state serialization is
the :smtk:`diagram panel <pqSMTKDiagamPanel>`, so that positions of task nodes
(as edited by users) can be saved and retrieved when writing and
reading projects that define those tasks.

Phrase Model Ignores Most Resource-Manager Events
-------------------------------------------------

With one exception in :smtk:`ResourcePhraseModel <smtk::view::ResourcePhraseModel>`,
the :smtk:`PhraseModel <smtk::view::PhraseModel>` classes now ignore resource-manager
events.
Instead, operation results are used to deal with the addition and removal
of resources to/from a resource manager.
This is consistent with the SMTK's paradigm: all modifications of resources and
components should take place inside operations and use the base
:smtk:`Operation <smtk::operation:Operation>` class to handle addition/removal of
resources to the application's manager.

If you have custom subclasses of PhraseModel, you should attempt to do the same.

As part of this change, the base PhraseModel class now provides a virtual
``processResource()`` method; it is invoked by the phrase-model's ``handleOperation()``
method when resources are added or removed by an operation.
Previously, both the ComponentPhraseModel and ResourcePhraseModel implemented methods
of the same name and signature that were **not** overrides of this new method in the
base class.
However, since they served the same purpose, they are now virtual overrides.
If you have custom subclasses of PhraseModel, you may wish to override this method
and you should guarantee that you do not hide the base-class method with a non-virtual
method of the same signature.

PhraseModel::triggerDataChanged() is now Rate-Limited
-----------------------------------------------------

Put `PhraseModel::triggerDataChanged()` on a timer in order to prevent overly frequent redraws by rate-limiting observers to be fired at most 10 times per second.

Phrase Model Batches Triggers to Remove Phrases
-----------------------------------------------

Previously, ``smtk::view::PhraseModel::removeChildren()`` would invoke
observers once for each phrase to be removed. Now, when phrases to be
removed are consecutive, a single invocation of the observers is
performed with a range of child indices. As long as your phrase-model
observers can handle ranges, no change should be required on your
part and performance should be improved for large removals.


SMTK UI Related Changes
=======================

Added Support for Units in DoubleItem Editor
---------------------------------------------

The default line editor for double items specified with units was changed to
include units in the text input, e.g., "3.14159 ft" or "2.71828 m/sec".
The new editor includes a dropdown completer listing the compatible units
for the item.
The list of compatible units are obtained from the unit system stored in the
attribute resource.
Double items specified without units use the same editor field as before.
Double items specified with units that are not recognized by the units system
also use the same editor as before.
Items with discrete options also use the same editor as before.

Where the new units-aware UI is used, the label no longer includes the units
string. It is replaced with a placeholder string when the field is empty
and the units completer otherwise.

.. image ../images/UnitsUI.png

An example template file can be found at data/attribute/attribute_collection/unitsExample.sbt.

The results display for infix expressions was also updated to append the units string
for the case where the units are recognized by the units system.

Ternary Visibility Badge Support
--------------------------------------

Created two new badges - one for controlling a phrase's geometric visibility
and another for controlling / displaying the visibility of the phrase's
children, i.e. hierarchy.

The :smtk:`geometric visibility badge <smtk::extension::paraview::appcomponents::GeometricVisibilityBadge>`
is binary and can have the following values:

* *Visible* if the object's geometry is visible
* *Invisible* if the object's geometry is invisible

The :smtk:`hierarchical visibility badge <smtk::extension::paraview::appcomponents::HierarchicalVisibilityBadge>`is
ternary and can have the following values:

* *Visible* if all of its children are visible
* *Invisible* if all of its children are invisible
* *Neither* if some of its children are marked *Neither* and/or not all of its children are visible

The user can set all of the phrase's descendants' visibilities by toggling the Hierarchical Badge.

**Note:** The geometric visibility badge of phrase that corresponds to a resource will
effect the ParaView representation's Visibility.

**Note:** The hierarchical visibility badge of an object does not affect the geometric visibility of the object itself.

Added ReadOnly Mode to qtUIManager
----------------------------------

Added API to set the qtUIManager to be read-only.  Setting this to be true will
tell all the Views being displayed by the manager that no modifications should be
allowed.

This mode has been added to support the task system, where modifications need to be disallowed when a task is not active or marked completed.

Developer changes
~~~~~~~~~~~~~~~~~~

Added the following methods:

qtUIManager::setReadOnly(bool val)
bool qtUIManager::isReadOnly() const

Changes to qtUIManager's Error Reporting
----------------------------------------

Replaced uses of cerr with Logger in qtUIManager
Operation Toolbox
-----------------

The qtOperationPalette widget now accepts an "AlwaysFilter" configuration
parameter; when absent or false, a checkbox (labeled "All") appears in
the controls and can be used to display every registered operation without
any decoration.

New Project Shortcut Changed
----------------------------

Previously, `Ctrl+P` (`Cmd+P` on macos) would create a new project.
However, ParaView uses this key sequence for point-picking in render windows,
so now `Shift+Ctrl+P` (`Shift+Cmd+P` on macos) is used to create new projects.

Diagram View
------------

SMTK now provides a new :smtk:`view <smtk::view::BaseView>` subclass
named :smtk:`smtk::extension::qtDiagram`.
See the Qt extensions section of the user's guide for more information.

Changes to Task UI Architecture
-------------------------------

Task panel classes have changed names
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``pqSMTKTaskDock`` and ``pqSMTKTaskPanel`` classes have been
renamed ``pqSMTKDiagramDock`` and ``pqSMTKDiagramPanel``, respectively.
This was done to reflect their new, more general purpose.
A placeholder class for the panel has been added; it should generate
warnings if you attempt to use it.

Subclassing task nodes
~~~~~~~~~~~~~~~~~~~~~~

Added the ability to assigned different types of qtTaskNodes to different tasks by making the following changes:

1. The original qtTaskNode class has been split into the following classes:
  * qtBaseTaskNode - an abstract base class from which all qtTaskNodes are derived from
  * qtDefaultTaskNode - an non-abstract class that functions as the original qtTaskNode class did.
2. Added the concept of a qtManager.  This is class's current role is to provide a qtTaskNodeFactory where plugins can added new qtTaskNode classes and the qtTaskEditor can find the appropriate qtTaskNode class for a specific Task.

To specify a qtTaskNode class for a Task, you can add the information in a Task Style as shown here:

    "styles": {
      "editPhysicalPropertyAttributes": {
        "attribute-panel": {
          "attribute-editor": "Physics"
        },
        "task-panel": {
          "node-class": "smtk::extension::qtDefaultTaskNode1"
        }
      },

qtTaskNode classes are specified w/r to the task-panel.

An additional qtTaskNode class : qtDefaultTaskNode1 has also been added as an example of creating a new qtTaskClass.  In this case, the window of the qtTaskNode is colored based on its state and activity.

Closing projects
~~~~~~~~~~~~~~~~

The task-manager user interface was not cleared when a project was closed.
This was due to the fact that the project manager's observers were not being invoked.
However, once this was corrected, several changes were required to
:smtk:`qtTaskEditor <smtk::extension::qtTaskEditor>` and
:smtk:`qtBaseTaskNode <smtk::extension::qtBaseTaskNode>` to properly remove nodes
and arcs from the scene:

+ Task nodes should not attempt to remove themselves from the scene inside their
  destructor; that is too late since overridden virtual methods to obtain their
  bounding boxes cannot be called from the destructor (causing a crash).
+ Instead, when the task editor wishes to clear the scene, it instructs
  the scene to delete items and then deletes its internal references to the items.

Task-node state consistency
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :smtk:`smtk::extension::qtBaseTaskNode`'s ``updateTaskState()`` method
now takes an additional parameter indicating whether the task is active or not.
This prevents an inconsistency between the UI and the task manager because
this function is called during transitions between active tasks;
since it was called for both nodes during the transition, ``isActive()``
would only return the proper result for one task.

Now, ``updateTaskState()`` is invoked by the :smtk:`smtk::extension::qtTaskEditor`
as it observes active-task transitions.

Finally, the task node initiates transitions in the active task (as the
user clicks on the title bar) but waits for a call from the editor to
update its user interface.

Task-node modifications
~~~~~~~~~~~~~~~~~~~~~~~

Now :smtk:`smtk::extension::qtBaseTaskNode` has a ``updateToMatchModifiedTask()``
method called whenever an operation modifies a task. It is up to the node to
ensure its visual representation (but not its incoming/outgoing arcs) are
up-to-date with the modified task. Usually, this is just ensuring the label
matches the task's name/title.

Task-editor interaction modes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :smtk:`smtk::extension::qtTaskEditor` has been refactored so that each
user-interaction mode is a separate class that installs event filters on
the :smtk:`smtk::extension::qtDiagramView` and adds a QAction to the task-panel's
toolbar. The QAction is checkable and, when triggered, causes the editor to
enter its corresponding interaction mode.
All the modes' actions belong to a ``QActionGroup`` so that only one action
may be selected at a time.

The task editor now provides 4 modes: one for panning the viewport,
one for selecting task nodes, one for connecting nodes via arcs, and
one for removing arcs between nodes.

Arc editing
~~~~~~~~~~~

As discussed above, arcs may be created and removed via the task editor.
The task editor's :smtk:`smtk::extension::qtConnectMode` monitors the
:smtk:`smtk::operation::ArcCreator` operation group to discover what types
of operations exist to create arcs (and what type(s) of arcs each operation
can create) and launches the user-indicated operation to connect a selected
pair of nodes.

Arcs may be removed by entering the :smtk:`smtk::extension::qtDisconnectMode`
and clicking the backspace key with one or more arcs selected.
If no operation exists to delete a selected arc, no action is taken except
that an error is reported.

Task-node constructor
~~~~~~~~~~~~~~~~~~~~~

The class hierarchy and constructor for :smtk:`smtk::extension::qtBaseTaskNode` have changed:

* What was formerly the ``qtTaskEditor`` class has become :smtk:`smtk::extension::qtDiagram`.
* Now :smtk:`smtk::extension::qtTaskEditor` is a subclass of :smtk:`smtk::extension::qtDiagramGenerator`
  and is owned by a ``qtDiagram``. (This change was made to allow multiple sources of items to reside in
  the same overall diagram.)
* The constructors of nodes in the diagram all take :smtk:`smtk::extension::qtDiagramGenerator`
  rather than :smtk:`smtk::extension::qtDiagramScene` as the first argument to their constructor.
  This is because all nodes live in the same scene; diagram generators subdivide ownership more
  finely than the scene. Each node maintains a pointer to the diagram generator which created it.
* Task nodes no longer have a member variable named ``m_scene``. Instead, call the ``scene()``
  method on the node to obtain the scene from the diagram generator.

SMTK ParaView Extension Related Changes
=======================================

Representations have an Active Assembly
---------------------------------------

SMTK's custom ParaView representation now provides a string property
named ``Assembly``; this is required by new versions of ParaView so
that block selections (such as performed on context-menu clicks) will
work. On partitioned-dataset assemblies, the assembly name indicates
which (of possibly several) assembly hierarchy shold be used to identify
the selected blocks. For SMTK (currently a multiblock dataset), this
serves no purpose other than to present a crash when no such property
exists.

Closing Resources now Behaves Differently
-----------------------------------------

Previously SMTK's "File→Close Resource" menu item would close the
single resource whose ParaView pipeline source object was active.
This was problematic for several reasons:
+ Due to recent changes, not all SMTK resources have pipeline sources
  (particularly, those with no renderable geometry).
+ The user interface does not always make it clear which pipeline
  source is active (because modelbuilder hides the pipeline browser
  panel by default).
+ It was not possible to close multiple resources at once.

This has been changed so that
+ A set of resources is extracted from the SMTK selection (using the
  "selected" value label); all of these resources will be closed.
+ Because the SMTK selection is used, resources with no renderable
  geometry can be closed.
+ Closing a project now properly removes it from the project manager.
+ If resources are owned by a project, they will not be closed unless
  their owning project was also selected to be closed.
+ Users can choose whether to discard modified resources once (at
  the beginning of the process); if the user elects to save resources
  but then cancels during saving a resource, no further resources
  will be closed.
+ The :smtk:`pqSMTKSaveResourceBehavior` has been refactored to
  provide additional API that does not require ParaView pipelines
  as inputs; the original API remains.

Attribute Editor Panel fixes for the Task System
------------------------------------------------

Previously, if your project contained an attribute resource whose display
hint prevented it from being shown in the attribute editor panel at load,
then a task which requested one of its views to be shown as the task was
activated would fail to display it. This happened because the panel assumed
the current attribute resource contained the view configuration for the
new view. This has been fixed by searching for the specified view name
using the application's resource manager.

Changes to Operation Parameter-Editor Panel
-------------------------------------------

A new task style key ``hide-items`` was added to the panel for specifying
operation parameters to be hidden from the user interface when displayed.
The value is an array of strings, each specifying the path to one item
in the operation parameters.

Shallow Copy Changed
--------------------

The implementation of ShallowCopy for composite datasets changed in VTK such
that SMTK's ``COMPONENT_ID`` information-key was not preserved, making UUIDs
unavailable to consumers of renderable geometry. We fix this by using the
original ShallowCopy implementation, which was renamed to CompositeShallowCopy.

SMTK Python Related Changes
===========================

Python Scripts and Managers
---------------------------

New methods have been added to simplify scripting:

.. code-block:: python

   import smtk
   # Fetch a list of paths to SMTK plugins:
   pluginList = smtk.findAvailablePlugins()

   # Load a list of plugins:
   loaded, skipped = smtk.loadPlugins(pluginList)
   # If pluginList is omitted, all available plugins
   # will be loaded.

   # Fetch or create an application context:
   data = smtk.applicationContext()
   # (data will be an instance of smtk.common.Managers
   # initialized by calling all plugin registrars.)

These new methods work in both scripts and in ParaView-based
applications inside the interactive Python shell.
In the former case, the application context will be obtained
from the active client-server connection's SMTK wrapper object.
In the latter case, a :smtk:`smtk::common::Managers` object
will be created and initialized the first time the function is
called.

Python Operations can now access SMTK Managers and Projects
-----------------------------------------------------------

Python bindings were added so that Python operations can now retrieve the
``smtk::common::Managers`` object by calling ``smtk::operation::Operation::managers()``.
Individual manager instances can be retrieved from the managers object by calling
a new ``get()`` method and passing in the fully qualified class name of the object
to return. For example, to retrieve the project manager from the
``operateInternal()`` method:

.. code-block:: python

  project_manager = self.managers().get('smtk::project::Manager')


The ``smtk::project::Manager`` class was updated to add a ``projectsSet()`` method
so that Python operations can retrieve the projects for a given manager. The C++
methods returns a ``std::set<smtk::projectProject>`` and the Python binding returns
a Python set object.

SMTK Third-Party Related Changes
================================

PEGTL v2.8.3
------------

SMTK will now build with either PEGTL v2.7.1 or v2.8.3.
Our continuous integration machines have had their superbuild
upgraded to v2.8.3 to fix an issue with parse-trees that would
lead to duplicate terminal nodes. Using an version of PEGTL
older than v2.8.3 could lead to incorrect units.
