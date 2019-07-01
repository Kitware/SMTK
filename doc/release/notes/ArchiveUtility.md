## Introduce archive utility

An SMTK Archive is a portable collection of files that are stored as a
single file on-disk. An archive is described by its filesystem path. Once
instantiated, a user can insert files into the archive,
serialize/deserialize the archive to/from disk, access a list of files in
the archive, and acquire file streams to these files by accessing them via
their name. An archive can be considered a directory containing files;
as such, each file in the archive must be assigned a unique path.
