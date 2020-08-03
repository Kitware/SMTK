Gallery
-------

SMTK's existing widgets are listed below with instructions for their use.

Point
^^^^^

The :smtk:`pqSMTKPointItemWidget` can be attached to an SMTK Double item
with 3 entries: the x, y, and z coordinates of a point in world coordinates.
When the widget is in its fully enabled state,
the 'P' and 'Ctrl+P' (or 'Cmd+P' on macos) keys may be used to pick
point placement in the render window (on any fully-opaque surface underneath
the mouse pointer when the key is pressed).
The latter version will choose the nearest vertex in the dataset rather than
the point on the pixel directly under the pointer.

.. findfigure:: widget-point.png
   :align: center
   :width: 95%

   The Qt widget (left) and 3-D widget (right) for editing point locations.

Line
^^^^

The :smtk:`pqSMTKLineItemWidget` can be attached to an SMTK Group item
2 Double children holding 3 values each. Each Double item specifies
one line endpoint's coordinates.
In the item's view configuration, specify Point1 and Point2 attributes
naming the names of the Group's children that represent these points.

.. findfigure:: widget-line.png
   :align: center
   :width: 95%

   The Qt widget (left) and 3-D widget (right) for editing line segments.

Plane
^^^^^

The :smtk:`pqSMTKPlaneItemWidget` can be attached to an SMTK Group item
with 2 Double children holding 3 values each:
one specifying an Origin point and
the other specifying a Normal vector.

.. findfigure:: widget-plane.png
   :align: center
   :width: 95%

   The Qt widget (left) and 3-D widget (right) for editing (unbounded) planes.

Box
^^^

The :smtk:`pqSMTKBoxItemWidget` can be attached to an SMTK Double item
with 6 entries in the order (xmin, xmax, ymin, ymax, zmin, zmaz).
It can also be attached to a group with Min, Max, and optionally Angles
items representing two corner points in space and a set of 3 Tait-Bryan
rotation angles.
Finally, the box widget also accepts a group containing a Center,
Deltas, and optionally Angles items representing the box centroid,
lengths along 3 orthogonal axes from the center to each face of the
box, and rotation angles as above.

.. findfigure:: widget-box.png
   :align: center
   :width: 95%

   The Qt widget (left) and 3-D widget (right) for editing boxes.

Cone (and Cylinder)
^^^^^^^^^^^^^^^^^^^

The :smtk:`pqSMTKConeItemWidget` can be attached to an SMTK Group holding
two Double items, each with 3 values, representing the bottom and top
points of a truncated cone; plus two more Double items, each with a single
value representing the radius at each point.
Alternately, a single radius Double item may be provided to accept only
cylinders.

.. findfigure:: widget-cone.png
   :align: center
   :width: 95%

   The Qt widget (left) and 3-D widget (right) for editing truncated cones.

Sphere
^^^^^^

The :smtk:`pqSMTKSphereItemWidget` can be attached to an SMTK Group holding
a Double item with 3 values representing the Center of
a sphere and another Double item with a single value
representing the Radius.

.. findfigure:: widget-sphere.png
   :align: center
   :width: 95%

   The Qt widget (left) and 3-D widget (right) for editing spheres.

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

.. findfigure:: widget-spline.png
   :align: center
   :width: 95%

   The Qt widget (left) and 3-D widget (right) for editing polyline and spline curves.
