ParaView extensions
===================

3-d widget operation observer
-----------------------------

The :smtk:`pqSMTKAttributeItemWidget` class was monitoring operations to determine
when the attribute holding its items was modified.
It should not do this, instead relying on qtAttributeView or qtInstancedView to
call its ``updateItemData()`` method when an operation modifies the widget.
(It was already doing this.) Because its operation observer was not validating
that the operation causing changes was triggered by itself, a race condition
existed that could undo changes to one item controlled by the widget while
parsing updates from another item.
