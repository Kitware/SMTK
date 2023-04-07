ParaView extensions
-------------------

Closing resources now behaves differently
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
