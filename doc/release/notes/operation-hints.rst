Operation System
================

New render hint for object visibility
-------------------------------------

Operations can now add ``render visibility hint`` attributes
to their result to suggest to the application that resources
and/or components should have their visibility set to the
given value.

This is used by subclasses of :smtk:`smtk::task::EmplaceWorklet`
which create resources that have geometry which should not
be shown by default – but instead should only be shown when
the newly-created tasks become active.
