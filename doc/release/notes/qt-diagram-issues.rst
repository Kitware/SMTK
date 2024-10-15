Qt extensions
=============

Diagram view issues fixed
-------------------------

+ The `qtDiagramGenerator::updateScene()` method has been split into
  `qtDiagramGenerator::updateSceneNodes()` and `qtDiagramGenerator::updateSceneArcs()` so that arcs
  between nodes in different generators can be maintained properly. The diagram will call every
  generator's `updateSceneNodes()` method before calling generators' `updateSceneArcs()` methods.
  This way, creation of arcs can be delayed until all of the generators have created all new nodes.
  You should split your custom generator's `updateScene()` method into these two methods.
  Old code will continue to compile but will generate a runtime error message.
+ A crash when removing task nodes from ``qtTaskEditor`` was fixed.
+ The escape key now works to return to the default interaction mode
  from all of the ``qtDiagramViewMode`` subclasses.
+ Deleting arcs can be accomplished with either the delete or backspace
  keys (previously, only backspace worked).
+ Make shift in "select" mode temporarily enter "pan" mode to mirror
  the behavior of the same key in "pan" mode (which enters "select").
+ Replaced GridLayout which was causing the widget not to expand to consume all of the available space.
