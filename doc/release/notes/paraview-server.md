+ Some functionality in smtkPVServerExtPlugin has been split into
  new plugins named smtkPVModelExtPlugin and smtkPVMeshExtPlugin.
  If your application previously packaged smtkPVServerExtPlugin, you
  will want to consider packaging these new plugins. If you do, then
  previous functionality will be preserved. If not, then some operations
  from the mesh and model subsystem will no longer be present (including
  selection-responder operations) — assuming that other plugins did not
  introduce dependencies on smtk::model::Registrar or smtk::mesh::Registrar,
  respectively.

  As an example, omitting the new smtkPVMeshExtPlugin (in addition to
  the polygon-session, mesh-session, oscillator-session, and delaunay
  plugins) results in the mesh import operation being absent; users will
  not be prompted to choose a reader for file formats like STL, PLY, or
  OBJ files if the application provides other import operations that handle
  them. The reader-selection dialog can confuse users, so preventing multiple
  operations in the ImporterGroup from handling a file type is desirable.
