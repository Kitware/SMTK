## Changes to Mesh
### Preliminary support for higher order cells
Preliminary support for higher order cells has been added to SMTK's mesh
library. Currently, higher order meshes can be read from .vtu files, written to
.vtu files and visualized. All nonlinear cells are currently cast as Lagrange
cells whose order is dictated by the number of points that comprise the cell.
