## Introduce the Geometry System

SMTK now has a geometry subsystem.

We are working toward removing smtk::model::Tessellation in order
to resolve performance problems inherent in its design.
In its place is smtk::geometry::Geometry and subclasses that allow
developers an efficient (usually zero-copy) technique for providing
access to their geometry directly in the format that rendering
and analysis backends will use.

This first step simply adds smtk::geometry::Geometry but does
not force its use or remove smtk::model::Tessellation.
See `doc/userguide/geometry` for more information.

Additionally, resources may now associate any kind of VTK data
object with themselves or their components. Preliminary support
for rendering image data as 3 axis-aligned slices is now present
as a prototype (slice planes cannot yet be edited and only the
first image in a resource is rendered). In the future, this
support will be expanded.
