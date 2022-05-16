Qt subsystem: badge click actions
---------------------------------

The :smtk:`smtk::extension::qtDescriptivePhraseDelegate` class now
provides an action to badges when they are clicked.
Previously, when a user clicked a badge no information
on the current selection was passed to the badge.
This was because the delegate does not have access to
the view (and thus the Qt selection) in the method that
responds to clicks on badge icons.
Our solution is to provide a subclassed action (named
:smtk:`smtk::extension::qtBadgeActionSelectionToggle`), which
uses the SMTK selection rather than the Qt selection.


Developer notes
~~~~~~~~~~~~~~~

Note that badges may wish to test casting their action to
a qtBadgeActionSelectionToggle and call its method to visit
related objects rather than the parent-class's method to
visit related phrases; this is more representative of the
information available and is more efficient since otherwise
phrases that reference each selected object must be looked up.
