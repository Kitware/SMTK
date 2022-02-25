.. _release-notes-22.02:

=========================
SMTK 22.02 Release Notes
=========================

See also :ref:`release-notes-21.12` for previous changes.

SMTK Attribute Resource Changes
===================================

Added API to Disable Category Inheritance for Definitions
---------------------------------------------------------

Added the ability to turn off inheriting category information from the Base Definition. The mechanism to control this is identical to that used at the Item Definition level via the method setIsOkToInherit. This information is persistent and stored in both XML and JSON formats.

Developer changes
~~~~~~~~~~~~~~~~~~

New API:

* bool isOkToInherit() const;
* void setIsOkToInherit(bool isOkToInheritValue);

Attribute `itemAtPath()` extended for groups
--------------------------------------------

The attribute resource's method `itemAtPath()` has been extended to
recognize `N` as a path component representing sub-group number, where
`N` is the index of the sub-group. Python operation tracing will
generate paths using this notation. Sub-group items whose names are
integers must be preceded by the sub-group index to be retrieved
correctly.


SMTK QT Changes
=================

Base attribute view
-------------------

An unused method, ``qtBaseAttributeView::requestModelEntityAssociation()``,
has been removed.

Qt attribute class
------------------

Allow a qtAttribute's association item-widget to be customized.
Currently, this is accomplished by assigning the ``<AssociationsDef>``
tag a ``Name`` attribute and referring to it in the attribute's
``<ItemViews>`` section. This may change in the future to eliminate any
possible naming conflict with other item names.

Qt attribute item-information
-----------------------------

The qtAttributeItemInfo class now accepts shared pointers by const-reference
rather than by reference so that automatic dynamic-pointer-casts to
superclass-pointers will work.

New reference-tree item widget
------------------------------

Add a new qtReferenceTree widget that subclasses qtItem.
Combined with the change to qtAttribute described
above, this widget provides an alternative to qtReferenceItem
for attribute association; it takes more screen space but
provides better visual feedback.

Removal of qtActiveObjects
--------------------------

The ``qtActiveObjects`` class is no longer in use anywhere and has been removed.
If you used it in a third-party extension, you'll need to create a new object
to hold an ``smtk::view::Selection::Ptr`` and an active ``smtk::model::Model``.

SMTK ParaView Extensions Changes
===================================

ParaView and Qt operation panels and views
------------------------------------------

The SMTK operation panel is now a "legacy" panel.
It is not deprecated, but it is not preferred.
Instead, consider using the operation toolbox and
parameter-editor panels.
By having two panels that split functionality from
the previous one, users with small displays do not
have to share limited vertical space among the operation
list and operation parameters.
There are other improvements below.

SMTK provides two new operation panels:
+ An "operation tool box" panel that shows a list of
  operation push-buttons in a grid layout (as opposed
  to the previous flat text list).
+ An "operation parameter editor" panel than contains
  a tabbed set of operations. By using tabs, multiple
  operations can be edited simultaneously.

Because most applications will want to choose between
either the legacy panel or the new panels, there are
now separate plugins:

+ The ``smtkPQLegacyOperationsPlugin`` exposes the legacy
  operations panel in applications.
+ The ``smtkPQOperationsPanelPlugin`` exposes the toolbox
  and parameter-editor operations panels in applications.

Your application can include or omit any combination of
these plugins as needed.

See the SMTK user's guide for more information about the panels.

Fixed Crash when loading in multiple attribute resources
--------------------------------------------------------

Closing a CMB application that has more than one attribute resource loaded would cause the application to crash (even if all but one of the attribute resources were closed manually).  This commit fixes the problem by checking to make sure the attribute panel has not been deleted when observing changes.

Added the ability to set the View and Attribute Resource
--------------------------------------------------------
Both displayResource and displayResourceOnServer methods now take in optional view and advancedLevel parameters.  If the view is set, then it will be used to display the resource and the advance level in the UI Manager is set to advancedLevel, else the resource's top level view will be used and the advacedLevel is ignored.

3-D widgets
-----------

The pqSMTKPlaneItemWidget has been updated to use ParaView's
new display-sized plane widget.
The advantage of this widget is that it does not require
an input dataset with non-empty geometric bounds in order
to display properly (i.e., the plane can be thought of as
independent construction geometry rather than attached to
any particular component).

A new pqSMTKFrameItemWidget is available for accepting
orthonormal, right-handed coordinate frames from users.
It requires 4 double-valued items, each holding a 3-vector:
an origin, an x-axis, a y-axis, and a z-axis.

A new pqSMTKSliceItemWidget is available for displaying
crinkle-slices of data. This widget uses the new ParaView
plane widget and adds ParaView filters to render crinkle-
slices of a user-controlled subset of components.
This widget uses the new qtReferenceTree along
with a modified MembershipBadge (both described below).

A new pqSlicePropertyWidget class is used by the slice-item
widget above.

All of the existing 3-d widgets have been moved out
of the ``plugin`` subdirectory and their symbols are now
exported.
This change was made so the slice-widget could reference
methods on the plane-widget.

A new smtkMeshInspectorView class has been added; it is
a custom attribute view for the mesh-inspector operation
described below.

SMTK VTK Extension Changes
==========================

Mesh inspection operation
-------------------------

Add a MeshInspector "operation" for inspecting meshes to
SMTK's VTK extensions.
The operation does nothing; its parameters will have custom
widgets that add mesh inspection to the operation panel.
It can be applied to any component with geometry.

SMTK StringUtils Changes
========================

Added toBoolean to StringUtils
------------------------------

The new method will convert a string to a boolean.  If the method was successful it will return true else it will return false.  All surround white-space is ignored and case is ignored.

Current valid  values for true are: t, true, yes, 1
Current valid  values for false are: f, false, no, 0

SMTK Graph Session Changes
==========================

Graph Arcs/OrderedArcs use WeakReferenceWrapper
--------------------------

The type used for storing arc ``ToTypes`` has been changed from a
``std::reference_wrapper<ToType>`` to a new type
``smtk::common::WeakReferenceWrapper<ToType>`` in order to communicate to the
arc that the node component of the edge has been removed. This allows nodes to
be removed and edges to update lazily.

Developer changes
~~~~~~~~~~~~~~~~~~

Plugins using SMTK graph infrastructure will need to rebuild and fix type errors
to match the new implementationn in SMTK. They will also need to make sure they
are checking if ``to`` nodes in an arc are valid using the
``smtk::common::WeakReferenceWrapper<ToType>::expired()`` API to dectect if the
node is expired or not before accessing it.

Previously, if a node was removed, access via a ``to`` node in an arc would
silently fail or sefault. Now, invalid access will result in a
``std::bad_weak_ptr`` exception when attempting to access the expired data.

GraphTraits allows customized NodeContainer
-------------------------------------------

Allow implementor to override the default container used for storing
node data.

NodeContainer API
~~~~~~~~~~~~~~~~~~

Developers may now implement custom node storage as an option in GraphTraits.

Implementations of ``NodeContainer`` must implement a minimal API to access and modify
the underlying node storage. Additional public APIs will be inherited by the
``smtk::graph::Resource``.

.. code-block:: c++

  class MinimalNodeContainer
  {
  public:
    // Implement APIs inherited from smtk::resource::Resource.

    /** Call a visitor function on each node in the graph.
     */
    void visit(std::function<void(smtk::resource::ComponentPtr>>& visitor) const;

    /** Find the node with a given uuid, if it is not found return a nullptr.
     */
    smtk::resource::ComponentPtr find(const smtk::common::UUID& uuid) const;

  protected:
    // Implement protected APIs required by smtk::graph::Resouce and smtk::graph::Component.

    /** Erase all of the nodes from the \a node storage without updating the arcs.
     *
     * This is an internal method used for temporary removal, modification, and
     * re-insertion in cases where \a node data that is indexed must be changed.
     * In that case, arcs must not be modified.
     *
     * Returns the number of nodes removed. Usually this is either 0 or 1, however the
     * implementation may define removal of > 1 node but this may cause unintended behavior.
     */
    std::size_t eraseNodes(const smtk::graph::ComponentPtr& node);

    /** Unconditionally insert the given \a node into the container.
     *
     * Do not check against NodeTypes to see whether the node type is
     * allowed; this has already been done.
     *
     * Returns whether or not the insertion was successful.
     */
    bool insertNode(const smtk::graph::ComponentPtr& node);
  };

Developer changes
~~~~~~~~~~~~~~~~~~~

The Graph API no longer accepts storing nodes that are not derived from
``smtk::graph::Component``. This is enforced by the APIs required from the
NodeContainer.

Using the ``smtk::graph::ResourceBase::nodes()`` API is no longer available unless
the ``NodeContainer`` implements it.

The default ``NodeContainer``, ``smtk::graph::NodeSet``, provides an API for ``nodes`` and
is implemented using ``std::set<smtk::resource::ComponentPtr, CompareComponentID>`` as
the underlying container. This is consistent with the previous implementation and will
be automatically selected if no ``NodeContainer`` is specified in GraphTraits.

Note, a ``smtk::resource::ComponentPtr`` is used in the underlying storage to prevent having
to cast pointers for APIs inherited from ``smtk::resource::Resource``.

SMTK I/O Changes
=================

Logger severity rename
-----------------------

The `smtk::io::Logger` severity levels have been renamed:

  * `DEBUG` is now `Debug`
  * `INFO` is now `Info`
  * `WARNING` is now `Warning`
  * `ERROR` is now `Error`
  * `FATAL` is now `Fatal`

The existing names have been deprecated.

Adding Property Support in Attribute XML Files
----------------------------------------------

You can now set Properties on the Attribute Resource and on an Attribute via an XML file.

Property Section for the Attribute Resource
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
If you include a Attribute XML file that also assigns Resource Properties, the include file's Properties are assigned first.  Meaning that the file suing the include file can override the Properties set by the include file.

**Note** - the ability to unset a Property is currently not supported.

**Note** - Properties are currently not saved if you write out an Attribute Resource that contains properties in XML format.

SMTK View Changes
=================

Eliminating the need to call repopulateRoot
-------------------------------------------

Previously, handling new persistent objects forced the Component Phrase Model to
call repopulateRoot which can be very expensive for large models.  The new approach
identifies those Resource Components that would be part of the Phrase Model's root subphrases and
properly inserts them in sorted order, eliminating the repopulateRoot() call.

Resource lock badge
-------------------

There is now a "lock" badge that indicates when an SMTK resource
is locked by an operation. No visual distinction is made between
read or write locking. You may use this badge in any view that
displays resource phrases.

Membership badge
----------------

The membership badge has been extended to be pickier;
it is now possible to specify whether the badge should
apply to objects, resources, or components; whether
they must have geometry or not; and whether components
match a query-filter.

The purpose of these configuration options is to accommodate
operations that wish to pass geometry for several components
to be processed.



Python Related Changes
======================

Attribute Builder (Python)
--------------------------

A Python class :py:class:`smtk.attribute_builder.AttributeBuilder` was added
to support Python operation tracing. The class has a method
:py:meth:`build_attribute()` for editing attribute contents from an input
specification (dictionary). Details are provided at
`smtk/doc/userguide/attribute/attribute-builder.rst`.

Other Changes
=============

Plugin initialization based on ParaView version
-----------------------------------------------

Allow SMTK to create plugins for older versions of ParaView (e.g. v5.9) that
rely on Qt interfaces to invoke initialziation. These changes should not
require any changes to existing plugins using the `smtk_add_plugin` interface.
SMTK plugins that manually implement ParaView plugins via the
`paraview_add_plugin` CMake API should switch to using the SMTK wrapper for
creating SMTK plugins.


Generated Plugin Source:

* Generated sources for older versions or ParaView use an Autostart plugin
and are named using the scheme `pq@PluginName@AutoStart.{h,cxx}`. This will
include the Qt interfaces required for the ParavView Autostart plugin.

* Generated sources for newer versions of ParaView use an initializer
function, similar to VTK, and are named using the scheme
`smtkPluginInitializer@PluginName@.{h,cxx}`. This includes a function
that is namespace'd `smtk::plugin::init::@PluginName@` which is called by
ParaView.
