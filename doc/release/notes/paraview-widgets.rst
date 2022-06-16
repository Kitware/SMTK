3-D ParaView Widget API Change
------------------------------

Two virtual methods in :smtk:`pqSMTKAttributeItemWidget` now
return a ``bool`` instead of ``void``:
``updateItemFromWidgetInternal()`` and ``updateWidgetFromItemInternal()``.
The boolean should indicate whether the method made any changes
(to the item or the widget, respectively).
In the former case, the return value determines whether to emit a ``modified()``
signal (indicating the item has changed).
In the latter case, the return value determines whether to cause
the active view to be re-rendered (so that the widget is redrawn).
Previously, the methods above were expected to perform these tasks themselves
but now the base class uses this return value to eliminate redundant code.

Several 3-D widgets were missing an implementation
for ``updateItemFromWidgetInternal()``; this has been corrected.

Finally, when deleting 3-D widget items, a re-render of the active view
is forced. You may have noticed some occasions where widgets were
visible after deletion until the first redraw – that should no longer
occur.
