Qt extensions
=============

Diagram view issues fixed
-------------------------

+ A crash when removing task nodes from ``qtTaskEditor`` was fixed.
+ The escape key now works to return to the default interaction mode
  from all of the ``qtDiagramViewMode`` subclasses.
+ Deleting arcs can be accomplished with either the delete or backspace
  keys (previously, only backspace worked).
+ Make shift in "select" mode temporarily enter "pan" mode to mirror
  the behavior of the same key in "pan" mode (which enters "select").
+ Replaced GridLayout which was causing the widget not to expand to consume all of the available space.
