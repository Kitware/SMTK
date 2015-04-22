Key Concepts
============

Like the model system, the mesh system is composed of C++ classes,
also accessible in Python, and based on concepts from MOAB_
which it uses internally.
The class instances perform the following functions:

:smtk:`Handle <smtk::mesh::Handle>`
  instances refer to a single mesh entity, which may be an primitive
  mesh element (a :smtk:`Cell <smtk::mesh::Cell>`, e.g., a triangle
  or quadrilateral) or a set of mesh elements (a :smtk:`MeshSet`)
  grouped together for some purpose.

:smtk:`HandleRange <smtk::mesh::HandleRange>`
  instances refer to a range of mesh entities.
  A range is a run-length encoded list of mesh handles (which are opaque
  but internally represented by integers).
  By making HandleRanges a primitive type,
  meshes with implicit connectivity can be compactly represented.

:smtk:`MeshSet <smtk::mesh::MeshSet>`
  instances group :smtk:`HandleRange` instances together so that they
  can be marked with properties such as a common association with a model entity,
  material, boundary condition, or initial condition.
  Each MeshSet instance holds a reference to a parent collection (described below)
  and provides methods for setting attributes, computing subsets, and accessing
  primitive cells.

  In general, a MeshSet will not contain elements that overlap spatially.
  Instead, a meshset usually has a boundary that conforms to neighboring
  meshsets (or to empty space).
  Often, an SMTK modeling entity (corresponding to a
  :smtk:`CellEntity <smtk::model::CellEntity>`) will be associated
  with a meshset that approximates its point locus;
  however, not all MeshSets have an associated model entity.

:smtk:`Collection <smtk::mesh::Collection>`
  instances hold related MeshSets together.
  Problem domains are often the union of several MeshSets in a Collection.
  Often, the problem domain may be decomposed in several ways, so that
  all of the MeshSets in a collection may cover the problem domain several
  times over.
  For example, a Collection may have one MeshSet for each geometric model
  cell as well as a MeshSet for each material.
  Either of these alone would cover the entire problem domain;
  together, they cover it twice.

  All of the Cells in all of the MeshSets of a Collection have their
  connectivity defined by indices into the same set of points.

  Each Collection has a parent mesh Manager.

:smtk:`Manager <smtk::mesh::Manager>`
  instances contain mesh collections and provide an interface to an
  underlying mesh package, such as MOAB_ that implements methods to
  access the mesh.

.. _MOAB: https://bitbucket.org/fathomteam/moab
