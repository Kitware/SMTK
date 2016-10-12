Key Concepts
============

Like the model system, the mesh system is composed of C++ classes,
also accessible in Python, and is based on concepts from MOAB_,
which it uses internally. The paradigm we use facilitates an
interaction with meshes and their components in aggregate,
avoiding granular queries whereever possible (e.g. favoring
such set-theretical actions as "intersection" and "union", rather than
iterating over individual elements).

To enforce the concept of interacting with mesh elements in
aggregate, all mesh elements are represented as sets: there are no
classes that represent individual mesh elements (point, cell or
mesh). A user can interact with individual mesh elements via the
:smtk:`ForEach` interface, which iterates over single elements that
may be represented as slement sets containing a single element. It
should be noted that element-wise access to a mesh is usually not the
correct approach for most algorithms.

The class instances, listed hierarchically from the most granular to
the most aggregate, are as follows:

:smtk:`Handle <smtk::mesh::Handle>`
  instances refer to a single `smtk::mesh` entity, which may be a
  single primitive mesh element (a point or a cell, e.g., a triangle or
  quadrilateral),  a set of points (an :smtk:`PointSet`), a set of
  cells (an :smtk:`CellSet`), or a set of mesh elements (an
  :smtk:`MeshSet`) grouped together for some purpose.

:smtk:`HandleRange <smtk::mesh::HandleRange>`
  instances refer to a range of mesh entities.
  A range is a run-length encoded list of mesh handles (which are opaque
  but internally represented by integers).
  By making HandleRanges a primitive type,
  meshes with implicit connectivity can be compactly represented.

:smtk:`PointSet <smtk::mesh::PointSet>`
  instances group :smtk:`HandleRange` instances referring to points
  together so they can be marked with properties such as a common
  association with a model entity, material, boundary condition, or
  initial condition. As a set, each :smtk:`PointSet` supports set-theoretic
  actions (intersect, difference and union).   Each `PointSet`
  instance holds a reference to its parent collection (described below)
  and provides methods for setting attributes, computing subsets, and accessing
  individual points.

:smtk:`CellSet <smtk::mesh::CellSet>`
  instances group :smtk:`HandleRange` instances referring to cells
  together so they can be marked with properties such as a common
  association with a model entity, material, boundary condition, or
  initial condition. As a set, each :smtk:`CellSet` supports set-theoretic
  actions (intersect, difference and union).   Each `CellSet`
  instance holds a reference to its parent collection (described below)
  and provides methods for setting attributes, computing subsets, and accessing
  individual cells.

:smtk:`MeshSet <smtk::mesh::MeshSet>`
  instances group :smtk:`HandleRange` instances referring to meshes
  together so that they can be marked with properties such as a common
  association with a model entity, material, boundary condition, or
  initial condition. As a set, each :smtk:`MeshSet` supports set-theoretic
  actions (intersect, difference and union). Each `MeshSet` instance
  holds a reference to a parent collection (described below) and
  provides methods for setting attributes, computing subsets, and
  accessing individual meshes. A `MeshSet` also has access to its
  underlying `CellSet` and `PointSet`.

  In general, a MeshSet will not contain elements that overlap spatially.
  Instead, a meshset usually has a boundary that conforms to neighboring
  meshsets (or to empty space).
  Often, an SMTK modeling entity (corresponding to a
  :smtk:`CellEntity <smtk::model::CellEntity>`) will be associated
  with a meshset that approximates its point locus;
  however, not all MeshSets have an associated model entity.

:smtk:`Collection <smtk::mesh::Collection>`
  instances hold related MeshSets together.
  Problem domains are often the union of several instances of
  `MeshSet` in a `Collection`. Often, the problem domain may be
  decomposed in several ways, so that all of the `MeshSet`s in a
  collection may cover the problem domain several
  times over.
  For example, a `Collection` may have one `MeshSet` for each geometric model
  cell as well as a `MeshSet` for each material.
  Either of these alone would cover the entire problem domain;
  together, they cover it twice.

  All of the cells in all of the `MeshSet` instances of a `Collection` have their
  connectivity defined by indices into the same set of points.

  Each `Collection` has a parent mesh `Manager`.

:smtk:`Manager <smtk::mesh::Manager>`
  instances contain `Collections` and provide an interface to an
  underlying mesh package, such as MOAB_, that implements methods to
  access the mesh.

.. _MOAB: https://bitbucket.org/fathomteam/moab
