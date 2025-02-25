UI Changes
==========

Changes to qtItem::modified and the removal of qtItem::childModified
--------------------------------------------------------------------

qtItem::modified now will take the pointer to the qtItem that was modified.  In the original implementation, this Qt signal had no augments and the corresponding Qt slot would call the sender method to determine the qtItem that emitted the signal.  When qtGroupItem was added, it was requested that a second signal called childModified also be emitted to determine which child of the Group was actually modified.

Since then, other Items (notably ReferenceItem and ValueItem) have the ability to have children.  In addition, the previous childModified could not return qtItems that were more than 1 level below the item which was directly under the attribute itself.

This change does away which the childModified signal and instead now passes the qtItem that initiated the change.  If you still need to get the top-level item whose descendant is the one that was directly modified, you can still get the sender of the signal.

Developer changes
~~~~~~~~~~~~~~~~~~

* qtItem::modified(qtItem* item): now takes the modified qtItem as the argument.  To update existing code: *Q_EMIT this->modified()* use *Q_EMIT this->modified(this)*
* qtItem::childModified(qtItem *) has been removed - please use qtItem::modified instead.
