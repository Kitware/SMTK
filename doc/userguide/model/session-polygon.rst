Session: Polygon
----------------

SMTK has a session type named *polygon* that bridges Boost.polygon_'s modeling kernel.
This kernel provides access to 2-D polygonal models,
with operators that allow model edges and faces to be split or merged
as well as boolean operations and Voronoi diagram computation.

Boost's polygonal modeler uses integer arithmetic to achieve high performance
with robust geometric predicates.
SMTK converts 3-D floating-point coordinates into 2-D integer coordinates for you,
but you must provide several pieces of information for each model instance:

* A base point for the plane holding the polygon
* Either x- and y-axis vectors (in 3-D world coordinates) describing the planar
  coordinate system you wish to use, or an x-axis and the plane's normal vector.
* Either a minimum feature size (in world coordinates) that your model should
  represent or an integer model scaling factor that each world coordinate is
  multiplied by before rounding to an integer.

This session does not allow model edges to have more than 2 model vertices.
A model edge may have zero or one vertices when the edge is a periodic loop denoted
with identical first and last points; in this case, the first and last
point must also be the model vertex.
A model edge must have two model vertices when it is not periodic, one at each
end of the edge.
Model edges may have any number of interior points that are not model vertices.
These restrictions are imposed so that it is possible to quickly determine what
model face lies adjacent to each model vertex.
If model edges could have interior vertices,
the assumption that each edge may border at most 2 faces
would be much more difficult to enforce and validate.

This decision regarding model vertices and edges has further implications.
Edges may not have any self-intersections other than at points where segments meet.
When edges are joined into loops to form a face,
they are intersected with one another first;
if any intersections are found, then the model edges are split when
the face is created.

Note that SMTK is slightly more restrictive (in that it splits edges and
creates model vertices) than Boost requires because Boost does not model
edges at all; instead it models polygons as sequences of points –
optionally with a list of holes which are themselves polygons.
In Boost's modeler, points are not shared between faces;
each face is a model in its own right.
Because of this, it is simple for Boost to use *keyholed edges* to
represent holes in faces.
Keyholed edges are edges coincident along a portion of their length
and effectively split a face with holes into a face with no holes but
with infinitesimal slivers connecting the region outside the face to
each hole.
SMTK can accept keyholed edges but they must be broken into multiple
model edges at intersections so that SMTK's assumption that planar edges
border at most 2 different surface regions.

Meshing Boost.polygon models
============================

Boost polygonal models are conforming piecewise-linear cell complexes (PLCs), and
may thus be meshed by any SMTK mesh worker that accepts models in this form.

.. _Boost.polygon: http://www.boost.org/doc/libs/1_59_0/libs/polygon/doc/index.htm
