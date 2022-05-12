Integrating ParaView and SMTK Selection
---------------------------------------

Before we go further, it is important to understand what information ParaView
provides during the selection process and what events trigger the selection
process.

ParaView selection semantics
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are 3 basic output types for selections in ParaView:

1. Points – these are corners of cells or discrete spatial locations where glyphs
   are drawn;
2. Cells – these are spatial continuua defined by a set of corner points (although
   vertex cells are included even though they are isolated rather than continuous);
   and
3. Blocks (and/or entire datasets when the data is not composite).

Generally, ParaView will only allow one of these three output types at a time.
Besides the type of output, there are two approaches to selecting data that ParaView
provides in 3-D views:

1. Hardware selection – the framebuffer is rendered but instead of saving colors,
   the dataset ID, block ID, and point or cell ID are saved in special framebuffer
   objects. These IDs are used to create a ``vtkSelection`` data object. Thus
   hardware selections are always represented as ID-based selections (rather than
   frustum selections, etc.).
2. Software selection – A ``vtkSelection`` data object is created with a
   frustum using the current camera position. When representations process this
   type of selection, they use VTK filters to identify all the cells inside the
   frustum.

Different tasks that users perform in ParaView combine these two basic approaches
and three output types. Many (but not all) of these tasks are initiated via the
in-view toolbar shown in the figure below.

.. _pv-in-view-toolbar:

.. findfigure:: in-view-toolbar.*
   :align: center
   :width: 90%
   :alt: ParaView's in-view toolbar.

   The in-view toolbar directly above the render window holds buttons used to
   perform selections in ParaView. Each one combines the output type,
   selection apprach, and user intent in a different way but the same two
   functions in ParaView do all of the work.

* The 3-D view provides a context menu. If there is no pre-existing selection
  when the right mouse-button is clicked, ParaView selects what is underneath
  the mouse in order to determine the context.
* Clicking on the "change center of rotation" toolbar button followed by a
  click in a 3-D render view initiates a selection event to determine where
  to place the center of rotation.
* Clicking the in-view toolbar's block-selection button ("Select block")
  generates a rectangular block selection. This selects data from zero or more
  representations and, if those representations show composite data, particular
  blocks from them that overlap the given rectangle.
* Clicking the in-view toolbar selection buttons ("Select cells/points on/through")
  generates rectangular selection using either the hardware selector (when
  selecting "on" a surface) or a software selector (when selecting "through" the
  entire view frustum).
* Clicking the in-view toolbar polygon-selection buttons ("Select cells/points
  with polygon") generates a polygonal selection using the hardware selector.
* Clicking the in-view toolbar interactive selection buttons (both the
  "Interactive select cells/points on" and "Hover cells/points" buttons)
  generates a pixel-sized rectangular selection underneath the mouse as it
  moves.

Finally, it is important to understand how ParaView's client-server
mechanism is involved in selection.
Recall that the client is responsible for broadcasting commands based
on user input to the server(s), which the server(s) then perform.
The results from each server are combined (a.k.a. reduced) into a
single response sent to the client for display.
Selections thus originate on the client (where the user manipulates a
pointer/cursor inside a render-window) and are sent to the server
(where the GPU is used to render the selection as highlighted).
However, when hardware selection is involved, there is an additional
round trip to/from the server involved because – in order to create a
``vtkSelection`` holding the IDs of points, cells, or blocks being
selected – the client must ask the server(s) to render IDs to a framebuffer.

The ``vtkSelection`` data created by the client is distributed to each
server process and attached to each selected representation as an
additional input (beyond the pipeline source's data). It is up for each
representation to use the ``vtkSelection`` object as it sees fit to render
the selection.

ParaView vs SMTK
^^^^^^^^^^^^^^^^

ParaView/VTK and SMTK have different models of selection:

* ParaView and VTK represent selections as a tree of selection nodes,
  each of which may use different criteria (such as IDs, frustum bounds,
  or field data thresholds) to select points or cells or blocks. The nodes of
  the tree are then combined using boolean set-theoretic operations.
  The result is a set that may contain points, cells, and blocks (but
  ParaView currently expects only one of these at a time).
  ParaView has a global, application-wide selection.
* SMTK represents a selection as a map (not a set) from persistent objects
  (usually components) to integer values. The integer values are usually
  treated as bit vectors — each bit in the value is reserved for a
  different purpose.
  In modelbuilder, we currently use bit 0 for the "primary" selection and
  bit 1 to indicate a "hover" or "secondary" selection.
  SMTK allows multiple instances of :smtk:`smtk::view::Selection`, but
  the ParaView plugins that SMTK provides create a single instance for
  use on each ParaView server in order to harmonize with ParaView's global
  selection.
  This single instance is owned by the :smtk:`vtkSMTKWrapper` object
  on each server.

Because these are very different models, some care must be taken when
selections are made. What follows is a description of how selections are
handled:

* SMTK :smtk:`resource representations <vtkSMTKResourceRepresentation>`
  render their data using only SMTK selections; they ignore the ParaView/VTK
  ``vtkSelection`` inputs that are provided. This is done in order to allow the
  different integer values in SMTK's selection to affect how components are
  rendered.
* When a user creates a selection in ParaView's render window (or with
  ParaView's other tools such as the Find Data dialog), the VTK selection
  object is translated into an SMTK selection which replaces whatever
  value was previously present as SMTK's selection.
* Also, user selections in ParaView are _filtered_ selections; this means
  that components indicated by VTK's selection may not be directly added
  to SMTK's selection but instead may suggest related components for selection.
  This is done so that users can click on rendered faces but select the volume
  contained by the faces when the workflow requires a volume.
  Other suggestions are possible as well (e.g., choosing an entire group when
  a group member is selected, choosing an associated attribute when an
  associated component is selected).
  The :smtk:`pqSMTKSelectionFilterBehavior` class provides a toolbar for users
  and workflows to adjust what filtering is performed.
* When SMTK widgets change the SMTK selection, ParaView views containing
  SMTK representations should be re-rendered so that the updated selection
  is presented graphically.

Selection Translation
^^^^^^^^^^^^^^^^^^^^^

Because different resource types (e.g., mesh and model) may need to
interpret VTK selection nodes differently, SMTK uses operations to
translate VTK selections into SMTK selections.
The process is initiated on ParaView's client by
the :smtk:`vtkSMTKEncodeSelection` class.
This class inherits a ParaView class (``vtkPVEncodeSelectionForServer``)
used to translate VTK selections on the client into a set of
selection objects to be sent to each server on which some portion
of the selected data resides.
SMTK's application-components plugin registers a vtkObjectFactory override
so that when ParaView's selection changes and ParaView asks for a new
instance of vtkPVEncodeSelectionForServer, an instance of vtkSMTKEncodeSelection
is provided instead.

So, each time a selection is made, ``vtkSMTKEncodeSelection`` is instantiated
and its sole virtual method is called with a ``vtkSelection`` object.
This method invokes the parent method but also introspects the selection
object passed to it before it is modified;
each ``vtkSelectionNode`` at the top level of the ``vtkSelection`` object
represents a different representation involved in the user's selection.
The encoder loops over these top-level nodes and, when it finds SMTK representations,
it loops over operations in the :smtk:`VTKSelectionResponderGroup` of operations
that can be associated to the representation's resource until it finds one that
succeeds – indicating it was able to parse the VTK selection and
modify the SMTK selection accordingly.

All operations in the :smtk:`VTKSelectionResponderGroup` are expected to be
subclasses of :smtk:`RespondToVTKSelection`.
These operations should always modify the selection without notifying observers
(see the documentation for :smtk:`modifySelection <smtk::view::Selection::modifySelection>`
for how to do this) so that a single signal can be sent to selection-observers
after all operations have completed on all the affected resource selections.

The :smtk:`RespondToVTKSelection` operation provides block-selection translation
for any resource with tessellated or glyphed data. If all your application needs
is graphical selection of components, you do not need to implement a custom selection
responder. However, if your application wants to process selections of points or
primitives, you should implement a custom selection responder for your resource.
Typically, discrete geometric resources (i.e., those in which the model is not a
smooth spline surface, but a collection of triangles, quadrilaterals, and other
primitives) will want to provide users with a way to select its primitives.

The next section discusses a common use case for selections: creating a new model
entity whose geometry is some portion of an existing model entity.
This is often done when creating subsets or sidesets on which boundary conditions
or material parameters will be applied.

Ephemeral selections
^^^^^^^^^^^^^^^^^^^^

A common task in preparing discrete models for simulations is to partition their
primitives into regions representing different materials, boundary conditions,
initial conditions, or areas where different physical processes will be modeled.
Because users often need to iteratively edit these selections, it is useful to
create "ephemeral" components.
Ephemeral components are removed as soon as users are finised editing a selection.
