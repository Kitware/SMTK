# ParaView Extensions

## Widgets

The box widget now accepts a single DoubleItem (with 6 entries)
specifying an axis-aligned bounding box or a GroupItem
containing multiple DoubleItems that configure a bounding box
in different ways depending on how they are used.
See the pqSMTKBoxItemWidget header for details.

There is now an infinite cylinder widget, which can be bound to a Group
item containing 3 Double children that serve as a center point,
a direction vector, and a radius.
Note that the widget models a cylinder of infinite length cut
by a bounding box whose size is not currently specified.

There is now a cone-frustum widget which can be bound to a Group
item containing 3 or 4 Double children that server as endpoints
and endpoint radii. The cone has a special case for cylinders where
only 1 radius value is provided.
