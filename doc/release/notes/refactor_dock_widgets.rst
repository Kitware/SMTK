Refactor Dock Widgets
---------------------

Refactor smtk Panels so they inherit from `QWidget` instead of `QDockWidget`.
This will make it possible to reorganize them in client apps. By default
they will be added to a `QDockWidget` wrapper. This change is in progress.

First, move the Projects plugin to `extensions/paraview` and move all the
source files to a VTK module so they can be referenced externally.
