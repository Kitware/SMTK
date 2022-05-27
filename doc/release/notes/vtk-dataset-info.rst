Dataset information operation for VTK backend
---------------------------------------------

There is a new operation (:smtk:`smtk::geometry::DataSetInfoInspector`)
that computes, for each associated component, the number of points and
cells (of each type) present in the VTK-renderable geometry.

This operation has a custom view that automatically updates a table showing
the counts as soon as the associations change.
