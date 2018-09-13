Representations
---------------

In ParaView, each view that a user creates may display datasets in the pipeline
via a representation.
Formally, a representation is a vtkAlgorithm subclass that adapts data
for rendering based on the type of view:
a spreadsheet view uses a different algorithm to prepare data for display
than a 3-D render-view.
Besides adaptations for different view types,
representations may adapt different types of input datasets.
SMTK provides representations that adapt models, meshes, and
other resources for display in 3-D render views.
