## 3D point widget

The pqSMTKPointItemWidget now uses a custom subbwidget
(pqPointPropertyWidget) rather than relying on ParaView's
pqHandlePropertyWidget.  This widget is now much more compact
and includes the help string as a tooltip rather than a label.

Also, the point visibility checkbox is now a tri-state checkbox:
 + checked: the 3-d widget is visible and has keyboard shortcuts registered
 + partially checked: the 3-d widget is visible but no shortcuts are registered
 + unchecked: the 3-d widget is not visible
The point-visibility checkbox is optional — it will not be displayed unless
the item's View configuration includes `ShowControls="true"` —
and optionally ties its setting to a discrete string item
so you can save the visibility and interaction state as
part of the attribute system along with point coordinates.

### Limitations

This change does not provide an easy way to distinguish multiple
point-widgets in the 3-d scene yet, nor a way to force only 1 widget
at a time to register shortcuts for "P" and "Ctrl+P".
Thus, it is still possible for users to become frustrated when these
keys do not work simply because multiple widgets are visible.
