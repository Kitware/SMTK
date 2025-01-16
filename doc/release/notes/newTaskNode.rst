Qt Extensions
=============

Designed New qtTaskNode
-----------------------

This implementation only depends QGraphicsItem and does not use Qt Widgets.
The result is both a cleaner looking design and also does not suffer from the
transformation problems that was noticed on Macs.

The new qtTaskNode is composed of 3 main graphics items:

* An item to display the Task's state as a color
* An item to display and manage the Task's name as well as the Task's activation status
* An item to control the Task's completed state

Supporting the Snapping of Port Nodes
-------------------------------------

It is now possible to snap a Task's Ports to be close to the Task itself.
This is controlled by the following methods provided by qtDiagramViewConfiguration:

* snapPortsToTask - indicates that the port nodes should be snapped w/r the Task Node (on by default)
* setSnapPortsToTask
* snapPortOffset - provides the offset distance that the port nodes should use when offsetting (0 by default)
* setSnapPortOffset

Consolidating Color Information for Task and Port Nodes
-------------------------------------------------------

qtDiagramViewConfiguration now has light and dark mode palettes that are used by qtTaskPortNodes and qtTaskNodes.  It also provides the following methods:

* baseNodeColor() - represents the base color of a Task Node
* portNodeColor() - represents the color of a Port Node
* textColor() - represents the color used for text
* backgroundColor() - represents the color of the background
* borderColor() - represents the color used on the border of non-active non-selected Task Nodes
* colorFromToken() - returns the color associated with a string token.  If none is found, then a consistent (but not necessarily unique) color will be returned.

All of the above color methods will take light/dark mode into consideration.

Also qtDiagramViewConfiguration::colorForArcType has been deprecated since you can simply call colorFromToken.

Other Changes
-------------
All qtBaseNode is now derived from QGraphicsObject instead of deriving from both QObject and QGraphicsItem
