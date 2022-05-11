Reorganization of extensions/paraview/appcomponents
---------------------------------------------------

To benefit client apps of smtk, the sources and resources in
`extensions/paraview/appcomponents/plugin-core` have all been moved
to the `appcomponents` directory and into the `smtkPQComponentsExt`
library, so they are accessible outside smtk. The plugin has been
split into three, with `plugin-core` retaining the auto-start and
behaviors, and `plugin-gui` containing all the toolbars, and panels,
and `plugin-readers` containing the importers and readers.
