## Introduce VTK pipeline source for all smtk resources

All SMTK resources are represented in a VTK pipeline as a
vtkSMTKResource. This class holds a pointer to the SMTK resource and
can generate an appropriate converter to map the resource into a
vtkMultiBlockDataSet. The converter generation logic is extensible,
facilitating custom visualization pipelines for future resource
types.

ParaView-entwined resource access routines (file readers, file
importers, resource creation actions) derive from
vtkSMTKResourceGenerator, which derives from
vtkSMTKResource. Additionally, when a pipeline source is queried using
the associated resource as the key, a new pipeline source is now
generated in the event that a pipeline source cannot be
identified. This change facilitates the generation of ParaView
pipelines from any action that results in the creation of an SMTK
resource (in addition to the ParaView-style readers and resource
creation actions).
