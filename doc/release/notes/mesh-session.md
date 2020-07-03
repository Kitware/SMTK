# Mesh session

The mesh session now uses the new `smtk::geometry::Geometry` provider to supply
data for rendering.

The default mode for writing a mesh session model has been changed to archive the model
description and the mesh file into a single file. The `smtk::session::mesh::Write` operation
has a flag that can revert this functionality back to its original logic of writing multiple files.
