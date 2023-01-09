Resource read bugs
------------------

The :smtk:`smtk::resource::json::jsonResource` method had a bug
where a resource's location would be assigned its *previous*
location on the filesystem (i.e., where it was written to rather
than where it was read from). This behavior has been changed so
that the location is only assigned when the resource's
pre-existing location is an empty string.

Read operations are expected to set the resource's location to
match the filename provided to them.
The :smtk:`smtk::markup::Read` operation in particular has been
fixed to do this so that relative paths to geometric data are
now resolved properly.
