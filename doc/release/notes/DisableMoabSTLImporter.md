## Disable Moab STL Importer

An update to MOAB's lastest master caused the stl importer to fail. Until
this is fixed, we temporarily disable MOAB's stl reader (we still have
VTK's stl reader, if VTK is enabled).
