# ParaView Pipeline-Selection Synchronization

There is now a `pqSMTKPipelineSelectionBehavior` class that,
when instantiated by a ParaView plugin, updates the SMTK
selection when an SMTK resource is activated in the PV pipeline
browser and vice versa when an SMTK resource is selected and
has a pipeline-browser entry.

Previously, this functionality was provided by the
pqSMTKResourcePanel but is now split into a separate behavior.
