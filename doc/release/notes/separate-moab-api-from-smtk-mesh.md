#Separate MOAB API from smtk::mesh API

One of the primary components of `smtk::mesh` is
`smtk::mesh::HandleRange`, a range-based index set used to compactly
represent large numbers of indices. It was originally a typedef of
`::moab::Range`, and its MOAB-based API had begun to creep into other
non-MOAB sections of `smtk::mesh`. This class has been replaced by
boost's interval_set, which has similar functionality and allows us to
more cleanly contain MOAB's backend.

### User-facing changes

`smtk::mesh::HandleRange` now has a different API that is analogous to
its original version.
