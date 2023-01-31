.. _release-notes-22.11:

=========================
SMTK 22.11 Release Notes
=========================

See also :ref:`release-notes-22.08` for previous changes.


SMTK Platform and Software Process Changes
==========================================
Type-name now reported consistently across platforms
----------------------------------------------------

The :smtk:`smtk::common::typeName` templated-function now returns
a string that is consistent across platforms. Previously, MSVC
compilers would prepend "``class ``" (already fixed) or "``struct ``"
(newly fixed), unlike other platforms. They also report anonmyous
namespaces differently (``\`anonymous namespace'`` vs ``(anonymous namespace)``.
These are now adjusted so that type names are consistent.
This is required for python bindings for the graph-resource, which
use the reported type names of arcs (which may be structs and may
live in anonymous namespaces) when editing the resource.

If you previously had code that special-cased MSVC type names, you
should remove it.

Regex header wrapper
--------------------------

Add a new header to wrap the STL/Boost regex headers to ensure consistent usage of
the STL or Boost regex libraries. To force using Boost::Regex the cmake configuration
parameter SMTK_USE_BOOST_REGEX can be used.


SMTK Common Changes
===================

Type container now uses string tokens as keys
---------------------------------------------

Previously, :smtk:`smtk::common::TypeContainer` indexed an
object of type ``Type`` by ``typeid(Type`).hash_code()``.
However, on macos this caused issues where the hash code
varied depending on the library calling methods on the
type container. To fix this, the type container now
uses :smtk:`smtk::string::Hash` ids (as computed by
constructing a string Token) of the typename. This is an
internal change and does not affect the public API of the
template class; no developer action should be required.

String tokens are now constexpr
-------------------------------

The :smtk:`smtk::string::Token` class, which stores the hash
of a string rather than its value, now includes a ``constexpr``
hash function (the algorithm used is a popular one named "fnv1a")
so that strings can be tokenized at compile time. Previously,
the platform-specific ``std::hash_function<std::string>`` was
used.

This change makes it possible for you to use switch statements
for string tokens, like so:

.. code-block:: c++

   using namespace smtk::string::literals; // for ""_hash
   smtk::string::Token car;
   int hp; // horsepower
   switch (car.id())
   {
     case "camaro"_hash: hp = 90; break;
     case "mustang"_hash: hp = 86; break;
     case "super beetle"_hash: hp = 48; break;
     default: hp = -1; break;
   }

The hash algorithm will generate hashes of type ``std::size_t``
but only supports 32- and 64-bit platforms at the moment.
Note that because the string manager uses a serialization helper
to translate serialized hash values (this was previously required
since ``std::hash_function<>`` implementations varied), reading
tokens serialized by a 32-bit platform on a 64-bit platform will
not present problems. However, reading 64-bit hashes on a 32-bit
platform is not currently supported; it may be in a future release
but we do not foresee a need for it.

Note that if a string is tokenized at compile time (i.e., by
using ``"string"_hash`` instead of ``smtk::string::Token``'s
constructor), its string value will not be added to the
:smtk:`smtk::string::Manager` instance and can thus not be
retrieved at run time unless some other piece of code adds it.
Instead a ``std::invalid_argument`` exception will be thrown.

SMTK General Resource Related Changes
=====================================

Extended resource locking
-------------------------

Now any caller (not just the base Operation class) can acquire
resource locks via a new :smtk:`smtk::resource::ScopedLockSetGuard` class.
It has static ``Block()`` and ``Try()`` methods which accept a set of
resources to read-lock and a _set_ of resources to write-lock.
The method named ``Block()`` blocks until the locks are acquired
(NB: this may cause deadlocks if you are not careful).
The method named ``Try()`` does not block but may return a null pointer
to a ``ScopedLockSetGuard``.
If either method returns a non-null pointer, the referenced object's
destructor releases all the locks.
In no event will only a subset of resources be locked.

Extended resource locking
-------------------------

Now any caller (not just the base Operation class) can acquire
resource locks via a new :smtk:`smtk::resource::ScopedLockSetGuard` class.
It has static ``Block()`` and ``Try()`` methods which accept a set of
resources to read-lock and a _set_ of resources to write-lock.
The method named ``Block()`` blocks until the locks are acquired
(NB: this may cause deadlocks if you are not careful).
The method named ``Try()`` does not block but may return a null pointer
to a ``ScopedLockSetGuard``.
If either method returns a non-null pointer, the referenced object's
destructor releases all the locks.
In no event will only a subset of resources be locked.


SMTK Attribute Resource Changes
===============================

Changes to Copying Attributes and Assigning Attributes and Items
----------------------------------------------------------------

The old smtk::attribute::Resource::copyAttribute method has been deprecated by a
more flexible version that takes in three parameters:

* The Attribute to be copied
* A CopyAndAssignmentOption instance (this is a new class)
* A smtk::io::Logger instance

Much of the attribute "assignment" logic has been moved from the method to the new  smtk::attribute::Attribute::assign(...) method
which as the same signature as the copyAttribute method.

Similarly, the original smtk::attribute::Item::assign method has also been deprecated by a version that takes in the following parameters:

* The SourceItem whose values are to be assigned to the target Item
* A CopyAndAssignmentOption instance (this is a new class)
* A smtk::io::Logger instance

CopyAssignmentOptions class
~~~~~~~~~~~~~~~~~~~~~~~~~~~

This class represents three classes of Options:

* Copy Options controlling how an Attribute gets copied
* Attribute Assignment Options controlling how attribute values are assigned to another
* Item Assignment Options controlling how item values are assigned to another.

AttributeCopyOptions
^^^^^^^^^^^^^^^^^^^^
* copyUUID -  If set, this indicates that copied attributes should have the same UUID as the original.
  **Note** : the copying process will fail if the copied attribute would reside in the same resource as the original.

* copyDefinition - If set, this indicates that if the source attribute's definition (by typename) does not exist in the resource
  making the copy, then copy the definition as well.  This can recursively cause other definitions to be copied.
  **Note** : the copying process will fail if this option is not set and the source attribute definition's typename
  does not exist in the targeted resource.

AttributeAssignmentOptions
^^^^^^^^^^^^^^^^^^^^^^^^^^
* ignoreMissingItems -  If set, this indicates that not all of the source attribute's items must exist in the
  target attribute.  This can occur if the target attribute's definition is a variation of
  the source attribute's.
  **Note** : the assignment process will fail if this option is not set and if not all of the
  source attribute's items are not present in the target.
* copyAssociations - If set, this indicates that the source attribute's associations should be copied
  to the target attribute which will also take into consideration allowPartialAssociations
  and doNotValidateAssociations options.
* allowPartialAssociations - Assuming that copyAssociations option is set, if the allowPartialAssociations
  ** is not set ** then all of the source's associations must be associated
  to the target attribute, else the assignment process will return failure.
* doNotValidateAssociations - Assuming that copyAssociations option is set, the doNotValidateAssociations
  *hint* indicates that if it possible to assign the association information
  without accessing the corresponding persistent object, then do so without
  validation.

ItemAssignmentOptions
^^^^^^^^^^^^^^^^^^^^^
* ignoreMissingChildren - If set, this indicates that not all of the source item's children items must exist in the
  target item.  This can occur if the target item's definition is a variation of the source item's.
  **Note** : the assignment process will fail if this option is not set and if not all of the
  source item's children items are not present in the target.

* allowPartialValues - If set,  this indicates that not all of the source item's values must be
  copied to the target item. If this option ** is not set ** then all of the
  source item's values must be copied, else the assignment process will return failure.

* ignoreExpressions - If set, this indicates that if a source Value item that have been assigned
  an expression attribute, it's corresponding target item should be left unset.

* ignoreReferenceValues - If set, this indicates that a target Reference item should not be assigned
  the values of the corresponding source item.

* doNotValidateReferenceInfo - The doNotValidateReferenceInfo *hint* indicates that if it possible to assign a source Reference item's
  values to a target item without accessing the corresponding persistent object, then do so without validation.

* disableCopyAttributes - If set, this indicates that no attributes should be created when doing item assignments.
  An item assignment can cause an attribute to be created in two situations.

  First - A source Value item is set to an expression attribute that resides in the same
  resource and the target item resides in a different one.  In this case the default
  behavior is to also copy the expression attribute to the target item's resource and
  assign the copied attribute to the target item.

  Second - A source Reference item refers to an attribute that resides in the same
  resource and the target item resides in a different one.  In this case the default
  behavior is to also copy the referenced attribute to the target item's resource and
  assign the copied attribute to the target item.

Attribute update manager
------------------------

The attribute system now has an update manager to aid
you in migrating resources from one schema version
(i.e., template) to another.
See the :ref:`smtk-updaters` update factory documentation
for the basic pattern used to register handlers for
items, attributes, or even whole resource types.

As part of this change, each attribute resource now has
a ``templateType()`` and a ``templateVersion()`` method
to identify the schema from which it is derived.
The values are provided by the SimBuilder Template (SBT) file
used as the prototype for the resource.
Workflow designers should update template files with
a template type and version in order to support future
migration.

Reference item API addition
---------------------------

Now :smtk:`smtk::attribute::ReferenceItem` provides a ``numberOfSetValues()`` method
that returns the number of non-null items.

Reference Item
--------------

A bug in :smtk:`smtk::attribute::ReferenceItem`'s ``setNumberOfValues()`` method
sometimes modified an internal reference to the next unset but allocated value
pointing to a location that was unallocated. The next call to append a value
would then fail as the default append location was out of range. This has been
fixed and no action is necessary on your part. If you were observing failed
calls to append items to a ReferenceItem (or ComponentItem/ResourceItem), this
may have been the reason.


Changes to ValueItem's Expressions
----------------------------------

Improving the support of Value Items whose Expression are in a different Resource.

* Assigning a ValueItem to another now properly deals with this case
* Copying a ValueItem Definition will also now properly support this use case.

SMTK Markup Resource
====================

SMTK now provides a resource based on the :smtk:`smtk::graph::Resource`
for performing annotation tasks.
It is not fully functional yet, but can import image and unstructured
data, create analytic shapes, group geometric objects together, mark
objects as instances in an ontology, and more.
The objective of this resource is to allow more flexible conceptual models
compared to the :smtk:`smtk::model::Resource`.
We are working to extend the ontology editing capability to allow users to
edit arbitrary relationship arcs between objects in the resource.
Please see :ref:`smtk-markup-sys` for more details.


SMTK Graph Resource Changes
===========================

Graph resource arc-evaluators
-----------------------------

The graph resource's  ``evaluateArcs<>()`` method has
changed the way it invokes evaluators in two ways.

1.  It will always pass a pointer (or const-pointer, depending on
    whether the resource is mutable) as an argument to your evaluator.
    The resource pointer is passed just before any forwarded arguments
    you provide to ``evaluateArcs<>()`` and is included when invoking
    your functor's ``begin()`` and ``end()`` methods.
    Note that the resource pointer will appear **after** the arc
    implementation object passed to your functor's parenthesis operator.

2. The graph resource now accepts functors with no ``begin()`` or ``end()``
   method; you need only provide a functor with a parenthesis operator.

In order to migrate to new versions of SMTK, you must change your
functors to accept this resource pointer. If you were already passing
the resource in, you may wish to remove the redundant argument and
modify places where you invoke ``evaluateArcs<>()`` to avoid passing it.

Graph-resource Filter Grammar
-----------------------------

The query-filter string-parser for the graph-resource had
a bug where parsing would succeed with some incorrect grammars
because the parser was not forced to consume the entire string
to obtain a match; a partial match would succeed but not produce
a functor that evaluated graph nodes properly.
This has been fixed, so error messages should now be emitted
when a filter-string is ill-formed.


SMTK Operation Changes
======================

Operation-system Changes
------------------------

We continue to identify and eliminate inconsistent behavior in asynchronous operations.
At some point in the future, :smtk:`smtk::operation::Operation::operate()` will either
be removed or see its signature modified to no longer return an operation Result; the
expected pattern will be for all users to launch operations at that point and use an
observer or handler functor to examine the result (while resource locks are held by
the operation).

In the meantime, we have introduced a new way to invoke an operation: rather than
calling ``operate()``, which returns a result, you should either launch an operation
or invoke ``safeOperate()`` which accepts a functor that will be evaluated on the
result. The method itself returns only the operation's outcome. This is done to prevent
crashes or undefined behavior (that can occur if you inspect the result without holding
locks on all the involved resources). The handler (if you pass one) is invoked before
the operation releases resource locks. Note that ``safeOperate()`` blocks until the
operation is complete and should not be invoked on the main thread of a GUI application
since it can cause deadlocks.

Operation Hints
---------------

Operations can now provide hints to the application in their
results instead of directly interacting with application state.
Any operation-manager observers then have access to the hints
and can choose how (or whether) to process them based on
application state and user preferences. See the :ref:`operation-hints`
documentation for more details.


Coordinate transform editor
---------------------------

SMTK now provides an operation for editing coordinate transforms
to be applied to any component with renderable geometry. It works
by setting a :smtk:`smtk::resource::properties::CoordinateTransform`
property named `smtk.geometry.transform` on these components.
The SMTK ParaView representation recognizes this property and
renders components transformed.

The operation is named :smtk:`smtk::operation::CoordinateTransform`
and has a custom view that allows you to select a pair of coordinate
frames (the "from" and "to" location and orientation of a rigid-body
transform) that are already present as property values on any component.
You can also create and edit transforms in place, although edited
coordinate frame values are not currently saved – only the resulting
transform is. In the future, the "from" and "to" coordinate frames will
be saved along with the resulting transform to allow iterative editing
across sessions.

Assign colors operation
-----------------------

Previously, the AssignColors operation lived in the ``smtk::model``
namespace and could only be associated to model entities as it used
an API specific to model entities to set each component's color.
This operation has been generalized to use the base resource's
property API to set color and can thus be associated to any
persistent object. It now lives in the ``smtk::operation`` namespace.

New Operation Groups for Grouping and Ungrouping
------------------------------------------------

The operation subsystem of SMTK now has
a :smtk:`smtk::operation::GroupingGroup` and
a :smtk:`smtk::operation::UngroupingGroup`.
Operations registered to these groups are expected to
associate to group members or groups, respectively.

These groups will be used by ParaView extensions
in the future to create and destroy groups visually
rather than via the operation toolbox.

Property editor operation
-------------------------

SMTK now provides a freeform property editor
named :smtk:`smtk::operation::EditProperties`
you can use to both inspect and edit integer,
string, floating-point, and coordinate-frame
properties on any component.

The custom operation view monitors the SMTK
selection provided to the Qt UI manager for
changes and updates the set of components being
inspected/edited.




SMTK ParaView Related Changes
=============================

ParaView Group and Ungroup Menu Items
-------------------------------------

SMTK now includes a new plugin, ``smtkPQGroupingPlugin``, which
adds menu items to ParaView's "Edit" menu named "Group" and "Ungroup."
These menu items are used to create and remove groups for all
resources which have registered operations to the
:smtk:`smtk::operation::GroupingGroup` and
:smtk:`smtk::operation::UngroupingGroup`, respectively.
The menu items have shortcuts of ``Ctrl+G`` and ``Shift+Ctrl+G``,
respectively.

Note that currently only the markup resource provides operations
to the grouping/ungrouping groups.

Automated panel-switching fixed
-------------------------------

The move to separate panels from dockwidgets (cmb/smtk!2750) left code
that intended to show the current dock-widget dysfunctional (you must
call `→raise()` on the `QDockWidget`, not on the `QWidget` panel it
contains). This fixes situations, where the operation toolbox and the
parameter editor dock-widgets are tabbed together; selecting an
operation in the toolbox would not bring up the parameter editor panel.
Similarly, pressing Ctrl+Space (the "operation finder" shortcut) would
not bring up the operation toolbox.

Pipeline Source Signals
------------------------

The `pqSMTKBehavior` class now emits a Qt signal each time it creates
or deletes a `pqPipelineSource` instance corresponding to an SMTK
resource.


```
void pipelineSourceCreated(
  smtk::resource::Resource::Ptr smtkResource, pqSMTKResource* pipelineSource);

void aboutToDestroyPipelineSource(
    smtk::resource::Resource::Ptr smtkResource, pqSMTKResource* pipelineSource);
```

Resource Representation Color Fix
---------------------------------

An index out-of-range crash could occur when a component's "color" property
is set, but has fewer than 4 values.
Now the following is done:

+ 0 values is treated as the default color (white).
+ 1 value is treated as an opaque greyscale value.
+ 2 values is treated as a greyscale value plus opacity.
+ 3 values is treated as an opaque (red, green, blue) color.
+ 4 values is treated as before: (red, gree, blue, opacity).

Resource Representation Color Fix
---------------------------------

An index out-of-range crash could occur when a component's "color" property
is set, but has fewer than 4 values.
Now the following is done:

+ 0 values is treated as the default color (white).
+ 1 value is treated as an opaque greyscale value.
+ 2 values is treated as a greyscale value plus opacity.
+ 3 values is treated as an opaque (red, green, blue) color.
+ 4 values is treated as before: (red, gree, blue, opacity).


SMTK Python Related Changes
===========================

Python operations as modelbuilder plugins
-----------------------------------------

In the past, each time you ran modelbuilder and wanted to use
an operation written in Python, you would have to use the
"File→Import Operation…" menu item to register the operation.
Now you can add a small block to the bottom of your python
module and load it as a plugin in modelbuilder.
See :ref:`smtk-python-plugin` for the details.

SMTK 3D Widget Changes
======================

3-D ParaView Widget API Change
------------------------------

Two virtual methods in :smtk:`pqSMTKAttributeItemWidget` now
return a ``bool`` instead of ``void``:
``updateItemFromWidgetInternal()`` and ``updateWidgetFromItemInternal()``.
The boolean should indicate whether the method made any changes
(to the item or the widget, respectively).
In the former case, the return value determines whether to emit a ``modified()``
signal (indicating the item has changed).
In the latter case, the return value determines whether to cause
the active view to be re-rendered (so that the widget is redrawn).
Previously, the methods above were expected to perform these tasks themselves
but now the base class uses this return value to eliminate redundant code.

Several 3-D widgets were missing an implementation
for ``updateItemFromWidgetInternal()``; this has been corrected.

Finally, when deleting 3-D widget items, a re-render of the active view
is forced. You may have noticed some occasions where widgets were
visible after deletion until the first redraw – that should no longer
occur.


Cone-widget
-----------

The point-normal arrays created by the vtkConeFrustum did not have
names; now they are named "normals". If you had problems being unable
to find these arrays by name before, it should now be fixed.

SMTK UI Related Changes
=======================

Qt File Item Changes
--------------------

The QFileDialog created by qtFileItem now includes an "All supported types" entry
as the first set of file extensions when a :smtk:`smtk::attribute::FileItemDefinition`
is marked to accept existing files. On many platforms, this simplifies browsing since
users no longer have to select a specific file-type of interest before they are shown
all acceptable files.

This is achieved using a new ``FileItemDefinition::getSummarizedFileFilters()``
method that is available for you to use in your custom applications as well.

Qt UI for extensible file items
-------------------------------

Previously, the file-browser dialog for all filesystem items
– even extensible ones – only allowed you to select a single
file at a time. Now extensible filesystem items allow you to
choose multiple files and the qtItem subclass will add entries
as needed to hold your list of selected files (if possible).
