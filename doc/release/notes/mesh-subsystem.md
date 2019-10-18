## Changes to the mesh subsystem

### Cell Selection

Mesh cells can now be selected and used as input for operations. When
cells are selected, a meshset is constructed to contain the
selection. Once the selection is no longer referenced by any
operations, it is automatically deleted.

### Extract by Dihedral Angle

Given a 2-diensional triangular meshset, this operation traverses the
meshset and accumulates all neighboring cells whose dihedral angle is
less than a user-defined value.

### Extract Adjacency

Given a meshset and a desired dimensionality <d>, this operation computes
and returns a meshset containing the <d>-dimensional cells adjacent to
the input mesh.

### Extract Skin

Given a <d>-dimensional meshset, this operation computes a meshset
comprised of the <d-1>-dimensional exterior adjacency cells of the
input mesh.

### Point Locator implementation for models with mesh tessellations

An implementation of the abstract point locator class has been
implemented using Moab's point locator. This allows models that have
associated mesh tessellations to snap points to model surfaces without
having to load SMTK's ParaView extensions.
