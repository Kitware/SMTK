Remove Resources via Operation
------------------------------

The `Result` attribute of `Operation` now includes a `resourcesToExpunge`
item that tells the `Operation` base class to remove resources from the
resource manager after operation observers have run, and before releasing
resource locks, to avoid conflicts. The `RemoveResource`
operation now uses this mechanism, and `File .. Close Resource` uses
this operation instead of closing resources directly. The `Result`
attribute also includes a `removeAssociations` optional item which can
be enabled to remove the resource's links to other resources, to
prevent re-loading.

Any operations that remove resources should switch to this new pattern.

The `ResourceManagerOperation` sub-class is now redundant, so it is now
an alias of `XMLOperation`.
