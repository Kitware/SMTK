.. _release-notes-22.08:

=========================
SMTK 22.08 Release Notes
=========================

See also :ref:`release-notes-22.07` for previous changes.


SMTK Operation Changes
======================

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

Deprecate `ResourceManagerOperation`
------------------------------------

The `ResourceManagerOperation` sub-class is now redundant,
replace any occurrences with `XMLOperation`.

SMTK Software Process Changes
=============================

Continuous integration
----------------------

SMTK uses MSVC 2022 to test merge requests rather than MSVC 2019.
