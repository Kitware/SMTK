Fix Editing Descriptive Phrase Titles
-------------------------------------

When editing the title of a Descriptive Phrase of a Resource using the QT UI, the user is
presented with what was being displayed which included the Resource's name as well as with
Resource's location (in the case of the Resource Panel) or the Resource's Project role and name
(in the case of the Project Panel).

If the user didn't initially clear the title, the original title (including
the role or location information) was set as the Resource's new name instead of just the name component.

This has been corrected so that when editing the title, the user is only presented with the Resource's name.  When editing
is completed, the location/role information will be added properly.
