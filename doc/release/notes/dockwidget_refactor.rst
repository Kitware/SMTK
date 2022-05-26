Panels separated from DockWidgets
---------------------------------

Previously, several `Panel` classes derived directly from `QDockWidget`,
so they could be docked by the user in whatever arrangement was desired.

To allow re-use and rearrangement, these `Panel` classes now derive from
`QWidget`, and are placed inside a `pqSMTKDock<T>` which derives from
`QDockWidget`. `pqSMTKDock<T>` has a template parameter to allow it to create the
child `Panel` of the correct type. `Panel` classes must now implement `void
setTitle(QString title)` to provide the `pqSMTKDock<T>` with the correct title,
and use `setWindowTitle()` to provide the initial dock window title.
