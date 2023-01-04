ParaView plugins reorganized
----------------------------

SMTK's ParaView plugins have been reorganized into
+ a subset (``core``) which, while they may register new UI elements,
  do not introduce persistent user interface elements (panels, menus,
  toolbars) to ParaView's default interface.
+ a subset (``gui``) which do register new user interface elements
  that appear in ParaView-based applications by default.

This also involves splitting the auto-start class into two classes,
one for each set of plugins.

Going forward, if you add a plugin (or new functionality to an existing
plugin) please ensure you choose the correct target.
