PPG File Format
---------------

The SMTK polygon session includes an ``ImportPPG`` operation for creating 2-D
models from text file input. The ``ImportPPG`` operation is provided as a
convenience for exploring CMB's many capabilities as well as for testing,
debug, and demonstration.
The file format is a simple data-format that specifies 2-D geometry as a
list of vertex positions and polygon face definitions, with vertices
connected by straight-line segments.

.. include:: example1.ppg
   :literal:

This example produces a model with two faces, a parallelogram-shaped
face embedded in a polygon face that also contains a square hole.

.. findfigure:: ppg-example1.png

PPG Features
============

As a tool intended mainly for educational use, the supported feature set
is purposely limited.

* Each polygon face must have simple geomety, specifically, polygons
  cannot be self-intersecting, and polygons with more than four edges must
  be convex.
* Polygons may share edges but cannot otherwise intersect or overlap.
* Polygons may include inner edge loops as either holes or embedded faces.
  Inner edge loops may not share edges or vertices with other loops, and
  embedded faces may not contain inner edge loops themselves.
* The ImportPPG operation has only syntactic checking, so users are
  encouraged to check their input files carefully for geometric validity.

File Format
===========
A PPG file may contain comments, vertex data, and face elements.
Empty lines are ignored.

**Comments**

Anything following a hash character (#) is a comment.

**Vertices**

A model vertex is specified via a line starting with the letter
``v`` followed by the x and y coordinates.

**Face Elements**

Model faces are specified using a list of vertex indices. Vertex
indices start with 1. Each face is defined by three or more vertices.

* An model face is specified with the letter ``f``
  followed by an ordered list of vertex indices. ImportPPG will
  create a model edge between each pair of adjacent vertices in the
  list, and between the last and first vertices, to form a polygon.
* Inner edge loops can be specified in the lines immediately following the
  model face.
* Embedded faces are specified with the letter ``e`` followed by an
  ordered list of vertices.
* Holes in the model face are specified with the letter ``h`` followed
  by an ordered list of vertices.

**Shared Edges**

As noted above, model faces can be adjacent with a common internal edge
between them. Note that the vertices at either end of the common edge must
be included in the vertex list of each face.

.. include:: example2.ppg
  :literal:

.. findfigure:: ppg-example2.png
