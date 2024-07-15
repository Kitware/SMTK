Diagram views
-------------

The :smtk:`smtk::extension::qtDiagram` class is a type of view intended to display
schematics represented as nodes connected by arcs, using Qt's ``QGraphicsScene``
scene-graph API.

A single diagram view may show nodes and arcs for several schematics;
each schematic provides nodes and arcs to the view via a
:smtk:`smtk::extension::qtDiagramGenerator` subclass.
These are described in more detail below.

The diagram accommodates user interaction with its data via
a run-time-configurable set of interaction modes which
inherit :smtk:`smtk::extension::qtDiagramViewMode`.
These modes allow user activities such as panning; selection; and arc creation and deletion.
Only one interaction mode may be active at a time.
Modes are displayed to the user the view's title bar (if the view is in a ``QDockWidget``)
or a toolbar at the top of the view (if the view is contained in some other type of ``QWidget``).

The diagram provides a sidebar (also known as a "drawer" on macos) that may
be toggled on or off from the toolbar.
The diagram includes a legend at the top of the sidebar
showing the types of arcs present.
The legend may also show node types in the future.

.. findfigure:: diagram-panel.*
   :align: center
   :width: 95%

   The leftmost panel shows a qtDiagram view configured to display
   a task diagram (top) and a resource diagram (bottom). The sidebar
   on the left shows the legend and widgets specific to each
   diagram-generator.

The image above shows a panel containing a qtDiagram; note that
the selection is brushed across all three views (the diagram, the
center 3-d rendering, and the resource browser to the right).
At the top of the panel is the title bar showing, from left to right,
a button to control the sidebar visibility; the diagram title;
and four buttons representing interaction modes (which are in a
QActionGroup so only one may be active at a time).

Configuration
~~~~~~~~~~~~~

As with other view classes, the diagram class is configured by a
:smtk:`configuration <smtk::view::Configuration>` object owned by an
:smtk:`information <smtk::view::Information>` instance.

The top-level view configuration may specify the following attributes:

.. list-table:: Diagram view configuration attributes
   :widths: 15 30
   :header-rows: 1

   * - Attribute
     - Description

   * - Type
     - This value must be provided as ``Diagram`` to generate a diagram.

   * - Title
     - The text that should appear as the view's name. If you place
       this view inside a dock widget, it will be in the panel's title bar.

   * - Legend
     - Either ``true`` or ``false`` to indicate whether to show the legend.
       (Note the legend may be created and maintained even when false, but
       will not be shown if false.)

   * - SearchBar
     - Either ``true`` or ``false`` to indicate whether to provide a
       search bar to select objects in the diagram.

The children of the top-level :smtk:`configuration component <smtk::view::Configuration::Component>`
should be an ordered list of Generator and Mode elements.
Order is significant as it determines the order in which
user-interface elements are instantiated and iterated during operation.

Generators
~~~~~~~~~~

SMTK provides two generators:
one for task diagrams and one for showing the relationships
among all objects in managed resources.

Task diagrams
*************

Task diagrams guide users through workflows by showing
a set of available :smtk:`tasks <smtk::task::Task>` connected
by arcs indicating which tasks are dependent on others and
how information migrates through the workflow (via
:smtk:`task adaptors <smtk::task::Adaptor>`) as tasks are completed.

The task diagram places a list of :smtk:`worklets <smtk::task::Worklet>`
in the sidebar that may be dragged and dropped into the diagram
to modify the workflow.

Resource diagrams
*****************

Resource diagrams display all the persistent objects (resources and components)
managed by the application.
The objects are laid out in a circle and arcs connect pairs of
objects to show their relationships.
Components have an arc to their owning resource.
Graph nodes have arcs for each type of relationship
present in the graph.

.. findfigure:: resource-diagram.*
   :align: center
   :width: 95%

   An example resource diagram showing a markup resource and its components.
   Labels in italics point to the 3 types of nodes in the diagram while
   other labels indicate the description of each grouping node in the diagram.

The image above shows a resource diagram.
Rounded rectangles are drawn for grouping nodes (with no fill) and
component nodes (filled with the background color or the selection color
depending on their selection state).
Circles are drawn for nodes representing resources.

Arcs are drawn as splines that go up and then down a tree used to group nodes by type.
The "top" of each arc is the least-common ancestor node in the tree.
This type of layout is known as `Hierarchical Edge Bundling`_ after the 2006 paper
by Danny Holten.

.. _Hierarchical Edge Bundling: https://dl.acm.org/doi/10.1109/TVCG.2006.147

Currently, the tree used by the resource diagram to render arcs is the inheritance
hierarchy of the persistent objects being displayed.
For example, instances of :smtk:`smtk::attribute::Attribute` inherit
:smtk:`smtk::resource::Component` which inherits :smtk:`smtk::resource::PersistentObject`.
Thus, two attribute instances ``A`` and ``B`` will be siblings that share 3 common
ancestors (corresponding to the attribute, component, and persistent object classes).

Arcs are drawn with an opacity, O, proportional to the number of hops up and
down the tree described above; direct siblings like ``A`` and ``B`` have the
fewest number of hops. Nodes that are instances of other classes will have arcs
with more hops since the arc must traverse farther up the tree before descending.

.. math::

   O = O_{short} - O_{long} \left(1 - \exp\left(3 - N\right)\right)

where

* :math:`O` is the opacity of an arc with :math:`N` total hops (3 being the minimum since the source and target nodes are included as well as their common ancestor),
* :math:`O_{short}` is the opacity to assign to the shortest arcs, and
* :math:`O_{long}` is the largest *adjustment* to apply to :math:`O_{short}` as arc length increases.

The :math:`O_{short}` and :math:`O_{long}` opacities are configurable parameters
described below.

This diagram accepts several configuration options.
Besides the ``Type`` attribute in the ``<Diagram Type="smtk::extension::qtResourceDiagram"/>``
tag, you can provide the following attributes

.. list-table:: Resource-diagram generator configuration attributes.
   :widths: 20 30
   :header-rows: 1

   * - Attribute
     - Description

   * - Name
     - A human-readable name to be shown as needed. (Currently this is unused.)

   * - Beta
     - The Beta parameter specifies how "tight" arcs between nodes
       follow the control polygon (the path up the tree from the
       source node and down the tree to the destination node).
       Values must be in [0, 1] where 0 will result in straight
       lines between nodes while 1 will specify arcs whose control
       polygons exactly match the description above.
       The default value is 0.95.

   * - ShortArcOpacity
     - The opacity, :math:`O_{short}`, to use when coloring arcs that connect
       direct sibling nodes (the shortest possible arc).
       The default value is 0.9.

   * - LongArcOpacityAdjustment
     - The largest difference, :math:`O_{long}`, from the ShortArcOpacity to use.
       Since an exponential is used to compute the adjustment, this factor will
       likely never be exactly realized by a graph but long arcs will have :math:`O`
       arbitrarily close to :math:`O_{short} - O_{long}`.
       The default value is 0.2.

   * - NodeSpacing
     - This factor (which should be greater than or equal to 1 to prevent overlap)
       determines the radius of the circle such that nodes can be rendered with
       space between them. A value of 1 packs the nodes next to one another with
       no additional space. A factor of 2 will leave space equal to the size of a
       node between each pair of nodes.

In addition to the attributes above, child elements can provide additional configuration.

.. list-table:: Resource-diagram generator child elements for configuration.
   :widths: 20 30
   :header-rows: 1

   * - Child
     - Description

   * - ``ObjectFilters``
     - This specifies regular expressions of object types that should be
       blacklisted or whitelisted. This element may have an
       ``<Accepts>…</Accepts>`` and/or a ``<Rejects>…</Rejects>`` element
       as children.
       Inside the ``Accepts`` or ``Rejects`` elements, there may be any number
       of ``<Filter>…</Filter>`` tags specifying regular expressions to
       match to object type-names which should be accepted or rejected.
       Rejections are always processed first.

   * - ``ClassExclusions``
     - This specifies object types that should be omitted from the tree used
       to organize nodes (but whose instances are not omitted from the diagram
       entirely). Object types listed in ``<Exclude>…</Exclude>`` tags under
       this node will be removed from the tree by edge contraction.

Modes
~~~~~

SMTK provides four modes described in the table below.
You may list as many or as few modes as you wish in the view
configuration.
Each mode should be a ``<Mode Type="mode-name"/>`` tag in
the view configuration.
If you create multiple modes,
you should choose one to be the default mode by adding
a ``Default=true`` attribute to the ``<Mode />`` element.

.. list-table:: Diagram interaction modes
   :widths: 20 30
   :header-rows: 1

   * - Mode name
     - Description

   * - :smtk:`smtk::extension::qtPanMode`
     - Users may pan the view  by clicking and scrolling.
       Nodes (but not arcs) may be selected/deselected by clicking over the node.
       Clicking over an empty area will deselect all nodes.
       Pressing the delete or backspace key will delete persistent objects
       represented by the selected nodes.
       If the "connect" mode below is present, holding the shift
       key will switch to that mode until shift is released.

   * - :smtk:`smtk::extension::qtSelectMode`
     - Users may rubber-band select nodes (not arcs) by clicking and dragging the pointer.
       Clicking over an empty area will deselect all nodes.
       Pressing the escape key will switch to the default mode (if the default is not "connect.")

   * - :smtk:`smtk::extension::qtConnectMode`
     - Users may connect nodes via arcs by clicking on a "from" (source) node and then
       clicking on a "to" (destination) node. The type of arc created may be chosen via a
       combo-box in the view's toolbar.
       A preview of the arc to be created is shown, colored either green (indicating the
       arc may be created) or red (indicating the arc may not be created).
       Pressing the escape key will reset the "from" node (if one has been chosen) or
       switch to the default mode (if no "from" node is set and a default mode is provided).

   * - :smtk:`smtk::extension::qtDisconnectMode`
     - Users may rubber-band select arcs (not nodes) by clicking and dragging the pointer.
       Clicking over an empty area will deselect all arcs.
       Pressing the delete or backspace key will delete arcs if an operation has been
       provided that can remove the arcs.
       Pressing the escape key will switch to the default mode (if the default is not "connect.")

See `this configuration`_ for a complete example of a diagram-view's configuration holding several
modes and diagram generators.

.. _this configuration: https://gitlab.kitware.com/cmb/smtk/-/tree/master/smtk/extension/qt/diagram/PanelConfiguration.json
