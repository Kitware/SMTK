## Additional mesh extraction routines

We have added a new mesh utility named
`extractCanonicalIndices`. This functionality is useful for exporting
to mesh formats that describe mesh faces by the index and face ID of
the 3-D cell containing the face. Given a (D-1)-dimensional mesh whose
(D)-dimensional adjacencies are held in a (D)-dimensional reference
mesh, the utility will return the extracted cell index of the
reference mesh and the canonical index of the (D-1)-dimensional cell
with respect to its (D)-dimensional adjacency. The canonical index of
a cell is defined in Tautges, Timothy J. "Canonical numbering systems
for finite-element codes." International Journal for Numerical Methods
in Biomedical Engineering 26.12 (2010): 1559-1572.
