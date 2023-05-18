Attribute System
----------------

Signal operation
~~~~~~~~~~~~~~~~

The :smtk:`Signal <smtk::attribute::Signal>` operation now has a new
resource-item named `resourcesCreated` which can be used to indicate
new attribute resources that were created externally and should be
added to the application's resource manager (by the base operation
class once observers have fired).

As an example, this feature is used by the `unitBadge` test to
indicate that its programmatically-created attribute resource
should be added to the phrase model used to exercise the badge
system.
