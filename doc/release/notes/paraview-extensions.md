## Changes to the ParaView extensions

### Widgets

+ An issue caused by widgets being asked to update from item contents during user interaction was fixed.
  This fix requires a change to all subclasses of pqSMTKAttributeItemWidget; any override of either
  `updateWidgetFromItem()` or `updateItemFromWidget()` should be renamed to `updateWidgetFromItemInternal()`
  or `updateItemFromWidgetInternal()` (respectively).
  Now, the non-`Internal()` methods update a new internal `m_p->m_state` variable so the widget does not
  attempt updates caused by itself.
