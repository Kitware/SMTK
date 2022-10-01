Coordinate transform editor
---------------------------

SMTK now provides an operation for editing coordinate transforms
to be applied to any component with renderable geometry. It works
by setting a :smtk:`smtk::resource::properties::CoordinateTransform`
property named `smtk.geometry.transform` on these components.
The SMTK ParaView representation recognizes this property and
renders components transformed.

The operation is named :smtk:`smtk::operation::CoordinateTransform`
and has a custom view that allows you to select a pair of coordinate
frames (the "from" and "to" location and orientation of a rigid-body
transform) that are already present as property values on any component.
You can also create and edit transforms in place, although edited
coordinate frame values are not currently saved – only the resulting
transform is. In the future, the "from" and "to" coordinate frames will
be saved along with the resulting transform to allow iterative editing
across sessions.
