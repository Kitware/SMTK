Qt subsystem: badge click actions
---------------------------------

Previously, the :smtk:`smtk::extension::qtDescriptivePhraseDelegate`
generated badge clicks inside its ``editorEvent()`` method.
However, that method is not provided with access to the view in
which the click occurred and thus could not access the view's
QSelectionModel.
Now, each widget that uses the descriptive-phrase delegate
(:smtk:`smtk::extension::qtResourceBrowser`,
:smtk:`smtk::extension::qtReferenceItem`,
:smtk:`smtk::extension::qtReferenceTree`) installs an
event filter on its view's viewport-widget and passes
mouse clicks to ``qtDescriptivePhraseDelegate::processBadgeClick()``.

User-facing changes
~~~~~~~~~~~~~~~~~~~

Users will now see that:

+ clicks on badges in the widgets above will not change the Qt
  selection as they did previously,
+ clicking on the badge of a selected item in the widgets above
  will act upon all the selected items, and
+ clicking on the badge of an unselected item in the widgets above
  will only act on that item.

This behavior should be significantly more intuitive than before.

Developer notes
~~~~~~~~~~~~~~~

If you use qtDescriptivePhraseDelegate in any of your own
classes, you cannot rely on it to handle badge clicks itself;
instead you must install an event filter on your view-widget's
viewport (not the view widget itself) and pass mouse clicks
to the delegate along with the view widget.
