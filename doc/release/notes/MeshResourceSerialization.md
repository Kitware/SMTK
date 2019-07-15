## Operations to read and write mesh resources as smtk files

All SMTK resources have the ability to read to and write from a json
file to pickle their state. Since the native file format for the dat
of an SMTK mesh is a .h5 file, we have introduced operations that
read and write SMTK mesh resources as a json description that includes
a URL to the underlying .h5 file.
