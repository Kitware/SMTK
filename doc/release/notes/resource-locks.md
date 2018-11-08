# Operations apply a Write lock on Resources by default

Resources and Components passed into Operations as input parameters
are now locked with Write access privelages (only one Operation can
mainpulate the resource) by default.

### User-facing changes

`smtk::operation::Operation` now has an API for other Operations to
execute the operation without first locking resources. The API uses a
variant of the PassKey pattern to only expose this functionality to
other operations. `smtk::operation::Operation` also contains a virtual
method that marks input resources with a Write LockType and resources
included in returned results as modified.
