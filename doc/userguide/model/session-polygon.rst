Session: Polygon
----------------

SMTK has a session type named *polygon* that bridges Boost.polygon_'s modeling kernel.
This kernel provides access to 2-D polygonal models,
with operators that allow model edges and faces to be split or merged
as well as boolean operations and Voronoi diagram computation.

Boost's polygonal modeler uses integer arithmetic.
SMTK converts 3-D floating-point coordinates into 2-D integer coordinates for you,
but you must provide several pieces of information for each model instance:

* A base point for the plane holding the polygon
* Either x- and y-axis vectors (in 3-D world coordinates) describing the planar
  coordinate system you wish to use, or an x-axis and the plane's normal vector.
* Either a minimum feature size (in world coordinates) that your model should
  represent or an integer model scaling factor that each world coordinate is
  multiplied by before rounding to an integer.


.. _Boost.polygon: http://www.boost.org/doc/libs/1_59_0/libs/polygon/doc/index.htm
