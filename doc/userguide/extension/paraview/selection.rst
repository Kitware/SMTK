Integrating ParaView and SMTK Selection
---------------------------------------

ParaView/VTK and SMTK have different models of selection:

* ParaView and VTK represent selections as a tree of selection nodes,
  each of which may use different criteria (such as IDs, frustum bounds,
  or field data thresholds) to select points or cells. The nodes of
  the tree are then combined using boolean set-theoretic operations.
  The result is a set that may contain points and cells (but is usually
  either points or cells, not both).
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
  selection inputs that are provided. This is done in order to allow the
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
This class inherits a ParaView class (vtkPVEncodeSelectionForServer)
used to translate VTK selections on the client into a set of
selection objects to be sent to each server on which some portion
of the selected data resides.
SMTK's application-components plugin registers a vtkObjectFactory override
so that when ParaView's selection changes and ParaView asks for a new
instance of vtkPVEncodeSelectionForServer, an instance of vtkSMTKEncodeSelection
is provided instead.

So, each time a selection is made, vtkSMTKEncodeSelection is instantiated
and its sole virtual method is called.
This method invokes the parent method but also introspects the selection
object passed to it before it is modified.
When it finds SMTK representations, it loops over operations in
the :smtk:`VTKSelectionResponderGroup` of operations that can be
associated to the representation's resource until it finds one that
succeeds, indicating it was able to parse the VTK selection and
modify the SMTK selection accordingly.
All operations in the :smtk:`VTKSelectionResponderGroup` are expected to be
subclasses of :smtk:`RespondToVTKSelection`.
These operations should always modify the selection without notifying observers
(see the documentation for :smtk:`modifySelection <smtk::view::Selection::modifySelection>`
for how to do this) so that a single signal can be sent to selection-observers
after all operations have completed on all the affected resource selections.

The :smtk:`RespondToVTKSelection` operation provides block-selection translation
for any resource with tessellated or glyphed data.
