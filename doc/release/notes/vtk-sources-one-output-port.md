## Collapse VTK sources & representations to use one port instead of three

Originally, VTK sources for SMTK resources had three output sources:
one for components, one for instance prototypes and one for instance
placements. A single instance of the consuming
vtkSMTKResourceRepresentation would then ideally connect to these
three ports. ParaView's default behavior resulted in the creation of
an instance of the representation class for each port, however,
resulting in duplicate rendering of instance prototypes as components
(in addition to other rendering artifacts, such as z-fighting).

The new design passes component multiblocks, instance prototypes and
instance placements via a single port as a multiblock with three
blocks. The consuming representation now extracts each of the three
blocks and renders them appropriately.
