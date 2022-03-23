3D Widget Visibility
--------------------------

3D widgets have an application-controlled setting that allows them to remain
visible even if their Qt controls loose focus or are hidden. This overrides
the ParaView default behavior of only allowing one 3D widget to be visible at
a time.

Developer changes
~~~~~~~~~~~~~~~~~~

By default, widget visibility behavior is not changed. To enable for an
application, call `pqSMTKAttributeItemWidget::setHideWidgetWhenInactive(false);`
on startup

User-facing changes
~~~~~~~~~~~~~~~~~~~

When this setting is enabled, multiple 3D widgets can appear in a renderview.
Widgets that might overlap need a user control to hide the widget, to allow
easier interaction. The last widget to be shown will receive precendence for
mouse and keyboard events.
