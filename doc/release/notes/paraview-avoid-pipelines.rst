ParaView Pipeline Sources
-------------------------

Previously, a ParaView pipeline source would be created any time
a resource was added to the resource manager owned by SMTK's
ParaView integration. This has been changed so ParaView pipelines
are only created when the resource provides a VTK renderable
geometry cache (i.e., when
``resource->geometry(smtk::extension::vtk::geometry::Backend())``
returns non-null).

This change was made to support workflows where many non-geometric
resources are loaded and the overhead of having ParaView attempt
to render them becomes significant.
