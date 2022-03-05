Python API changes
------------------

Due to fixes in recent Pybind11 releases, some types are no longer hashable in
Python. This requires changes in the Python API to change some return types
from Python ``set()`` instances into Python ``list()`` instances. This is
because ``std::set`` is, by default, turned into ``set()`` on the Python side,
but ordered on the C++ side does not imply hashable on the Python side. The
following APIs that have changed include:

  * ``smtk.mesh.MeshSet.cellFields()``
  * ``smtk.mesh.MeshSet.pointFields()``

The following types are no longer hashable:

  * ``smtk.mesh.CellField``
  * ``smtk.mesh.PointField``
