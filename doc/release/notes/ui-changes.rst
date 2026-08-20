QT Changes
===========

qtComponentAttributeView
--------------------------

If there is only one Attribute Definition available then the view
will automatically create Attributes for each Resource Component that
are applicable and will not display a combo-box in the second column and
instead displays the sole Definition's type.

Displaying Attributes in Tabular Format
---------------------------------------
Added the following classes:

* qtAttributeTableView - a View that manages attributes of a specified type using a table-based approach
* qtAttributeTableModel - a Qt Model for attributes of a given type used by the above View
* qtAttributeTableDelegate - a Qt delegate used by the above classes

Attribute-table views accept an optional ``HideInactiveChildren`` boolean
configuration attribute. When enabled, selecting a row hides columns for
conditional children that are inactive for that row. The columns are updated
when a discrete value is edited and are restored when no row is selected.

Attribute-table views accept an optional ``AskToDelete`` boolean configuration
attribute that, if true, will use a modal popup dialog to confirm attribute deletions.
Otherwise, clicking the delete button will immediately delete all selected attributes.

When an attribute definition contains group items, either at the top level or
beneath a discrete item, selecting its table row displays a standard attribute
editor below the table. This makes group contents, including extensible groups,
editable while keeping the table and conditional-child columns synchronized.
The table and attribute editor are separated by a vertical splitter so users
can adjust how much space is allocated to each. The attribute-editor pane is
scrollable, allowing group contents to remain accessible at any splitter size.
The editor honors definition-specific inline ``ItemViews`` and named ``Style``
configuration on each ``AttributeTypes/Att`` entry, including configuration
inherited from base definitions.

The ``ColumnDisplay`` attribute controls which item columns are included:

* ``All`` (the default) includes every item supported by the table model.
* ``TopLevelNonGroup`` includes top-level items except group items.
* ``TopLevelDiscrete`` includes only top-level discrete items.
* ``UserSpecified`` includes the exact item paths listed under
  ``TableItems``.

For example:

.. code-block:: xml

   <View Type="AttributeTable" ColumnDisplay="UserSpecified">
     <AttributeTypes>
       <Att Type="BoundaryCondition"/>
     </AttributeTypes>
     <TableItems>
       <Item Path="name"/>
       <Item Path="boundaryType"/>
       <Item Path="boundaryType/temperature"/>
     </TableItems>
   </View>

``Path`` uses SMTK's slash-separated item-path syntax. ``Name`` is also
accepted as a convenience for top-level items. The attribute-name column is
always displayed.

Mutually-exclusive conditional children may share one logical column by using
a ``Column`` entry. The active child supplies the value and editor for each
row; when none of the candidates is active, the cell is shown as inactive.

.. code-block:: xml

   <TableItems>
     <Item Path="boundaryType"/>
     <Column Name="BoundaryValue" Label="Value">
       <Item Path="boundaryType/temperature"/>
       <Item Path="boundaryType/pressure"/>
     </Column>
   </TableItems>

Shared-column candidates must be scalar, non-extensible value items of the
same type. They must be direct children of the same discrete item, each must
be activated by at least one enumeration, and no enumeration may activate
more than one candidate. The attribute subsystem provides
``validateExclusiveConditionalItems()`` for this definition-level validation;
invalid shared columns are ignored with a diagnostic.

Edits made through either the table or the selected-attribute editor now
launch an ``smtk::attribute::Signal`` operation. Item edits include the
modified item's runtime path (including the active candidate for a shared
column); attribute-level changes report an empty item-path list.

Task-diagram port layout
------------------------

Automatic Graphviz layout now positions only top-level diagram nodes. Child
nodes, such as external task ports, retain coordinates relative to their
parent task and move with it. Arcs connected to child nodes are mapped to
their top-level nodes for layout purposes, preserving dependency ordering
without allowing Graphviz scene coordinates to be interpreted as
parent-relative port coordinates. Duplicate mapped arcs and arcs internal to
one top-level node are omitted from the Graphviz input.

Emplacing a task worklet now preserves the relative positions of its task
nodes and ports. The drop-point translation is applied according to the
coordinate system used by each diagram item:

* Task-node positions are scene coordinates and are translated.
* Internal task ports are independent scene items and are translated.
* External task ports are children of task nodes; their parent-relative
  positions are not translated.

Worklet layouts without a top-level task-node position no longer cause an
invalid centroid calculation; their drop location is ignored with a warning.

Task-editor project closure
---------------------------

The task editor now releases its project-specific state safely when a project
is expunged. It removes the active-task observer before clearing the active
task, then clears the task path and the worklet palette's parent task while the
project is still available through the operation result. Finally, it releases
its task-manager pointer before subsequent diagram updates are processed.

This ordering prevents active-task callbacks from attempting to obtain shared
ownership of tasks that are already being released, which previously could
raise a ``std::bad_weak_ptr`` exception when closing or switching projects.

Task-diagram display for projects without an active task
---------------------------------------------------------

When a project is created, or when a loaded project has no active task, the
application now brings the task-diagram panel forward. Newly created projects
also reset the task diagram to its root view so that all top-level tasks are
shown. Loaded projects retain any restored diagram navigation state.

Box-widget defaults
-------------------

The ParaView box item widget now displays a **Reset to Defaults** button when
all of the numeric attribute items defining its box have default values. The
button restores every bound coordinate and rotation value, updates the
ParaView widget proxy, and renders the box using the restored values.

Box item views may set ``HideResetBoundsWhenDefaults="true"`` to hide
ParaView's **Reset Bounds** button and **Take account of block visibility**
checkbox when the box has defaults. The option is false by default.

.. code-block:: xml

<View
  Item="box"
  Type="Box"
  HideResetBoundsWhenDefaults="true"/>
