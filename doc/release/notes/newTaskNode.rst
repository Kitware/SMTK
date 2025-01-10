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
This is controlled by the setSnapPorts and snapPorts methods on qtBaseTaskNode.
