# VTK session

The VTK session now uses the new `smtk::geometry::Geometry` provider
to supply data for rendering without copying. This is a breaking
change and introduces a new dependency on vtkSMTKSourceExt for both
its Geometry class (which `smtk::session::vtk::Geometry` inherits) and
the vtkResourceMultiBlockSource so that UUIDs can be stored uniformly
by the session and the VTK backend.

Sessions which inherit the VTK session will need to be updated:

+ All use of `smtk::session::vtk::Session::SMTK_UUID_KEY()` should be
  eliminated in favor of calls to `GetDataObjectUUID` or
  `SetDataObjectUUID` in vtkResourceMultiBlockSource.
+ Instead of calling `smtk::session::vtk::Session::addTessellation()` on
  a model entity, you should call
  `smtk::operation::MarkGeometry(resource).markModified()` on the entity
  instead.
