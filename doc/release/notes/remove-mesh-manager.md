## Remove Mesh Manager

+ smtk::mesh::Maanger is now removed from SMTK.
+ mesh Collections are now optionally managed by a resource manager
  This is a significant change.
  Previously, mesh collections were held by a mesh manager owned by a
  model resource. Now, mesh collections are freestanding and can be
  classified onto a model resource. The classification mechanism is
  handled internally using resource links.
