Widgets
-------

SMTK provides special widgets for interacting with
model and mesh resources inside ParaView.
These qtItem subclasses may be used by specifying
them in an attribute's View configuration.

Point
^^^^^

The :smtk:`pqSMTKPointItemWidget` can be attached to an SMTK Double item
with 3 entries: the x, y, and z coordinates of a point in world coordinates.

Line
^^^^

The :smtk:`pqSMTKLineItemWidget` can be attached to an SMTK Group item
2 Double children holding 3 values each. Each Double item specifies
one line endpoint's coordinates.
In the item's view configuration, specify Point1 and Point2 attributes
naming the names of the Group's children that represent these points.

Plane
^^^^^

The :smtk:`pqSMTKPlaneItemWidget` can be attached to an SMTK Group item
with 2 Double children holding 3 values each:
one specifying an Origin point and
the other specifying a Normal vector.

Box
^^^

The :smtk:`pqSMTKBoxItemWidget` can be attached to an SMTK Double item
with 6 entries in the order (xmin, xmax, ymin, ymax, zmin, zmaz).

Sphere
^^^^^^

The :smtk:`pqSMTKSphereItemWidget` can be attached to an SMTK Group holding
a Double item with 3 values representing the Center of
a sphere and another Double item with a single value
representing the Radius.

Spline
^^^^^^

The :smtk:`pqSMTKSplineItemWidget` may represent a polyline or a cardinal spline,
depending on whether its View configuration has a `Polyline`
attribute set to true or not.
The widget must be attached to an SMTK Group holding
a Double item with 6 or more values representing the
3-D coordinates of handle points and an Int item
interpreted as a boolean that indicates whether the
curve should be a closed loop or an open segment.
