ParaView Extensions
-------------------

3-D widgets
~~~~~~~~~~~

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

View system
-----------

Membership badge
~~~~~~~~~~~~~~~~

The membership badge has been extended to be pickier;
it is now possible to specify whether the badge should
apply to objects, resources, or components; whether
they must have geometry or not; and whether components
match a query-filter.

The purpose of these configuration options is to accommodate
operations that wish to pass geometry for several components
to be processed.

Qt Extensions
-------------

Base attribute view
~~~~~~~~~~~~~~~~~~~

An unused method, ``qtBaseAttributeView::requestModelEntityAssociation()``,
has been removed.

Qt attribute class
~~~~~~~~~~~~~~~~~~

Allow a qtAttribute's association item-widget to be customized.
Currently, this is accomplished by assigning the ``<AssociationsDef>``
tag a ``Name`` attribute and referring to it in the attribute's
``<ItemViews>`` section. This may change in the future to eliminate any
possible naming conflict with other item names.

Qt attribute item-information
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The qtAttributeItemInfo class now accepts shared pointers by const-reference
rather than by reference so that automatic dynamic-pointer-casts to
superclass-pointers will work.

New reference-tree item widget
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Add a new qtReferenceTree widget that subclasses qtItem.
Combined with the change to qtAttribute described
above, this widget provides an alternative to qtReferenceItem
for attribute association; it takes more screen space but
provides better visual feedback.

VTK Extensions
--------------

Mesh inspection operation
~~~~~~~~~~~~~~~~~~~~~~~~~

Add a MeshInspector "operation" for inspecting meshes to
SMTK's VTK extensions.
The operation does nothing; its parameters will have custom
widgets that add mesh inspection to the operation panel.
It can be applied to any component with geometry.
