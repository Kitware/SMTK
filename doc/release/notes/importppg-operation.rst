ImportPPG Operation
===================

An ``ImportPPG`` operation has been added to the polygon session
for creating model resources from a simple text file input.
The "ppg" (Planar PolyGon) file format is a simple data format
that specifies 2-D geometry as a list of vertex coordinates and
polygon face definitions, with vertices connected implicitly by
straight-line model edges. Documentation is in the "Session: Polygon"
section of the SMTK user guide.

The ``ImportPPG`` operation is provided as a convenience for exploring
CMB's many capabilities as well as for testing, debug, and demonstration.
To use this feature from modelbuilder, the "File -> New Resource" menu
item now includes an option "Polygon -> Planar Polygon Model from PPG
File".
