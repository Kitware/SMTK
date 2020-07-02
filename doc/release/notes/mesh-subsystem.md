## Mesh Subsystem Changes

### Geometry

The mesh subsystem now uses the new `smtk::geometry::Geometry` provider to supply
data for rendering.

The default mode for writing a mesh has been changed to archive the mesh description and the mesh
file into a single file. The `smtk::mesh::WriteResource` operation has a flag that can revert this
functionality back to its original logic of writing multiple files.
