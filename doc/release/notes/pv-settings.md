## ParaView Settings

SMTK now uses ParaView's user preferences dialog.

### User-visible changes

Settings for SMTK (and specifically, a setting to highlight
objects in 3D render-views when the mouse is hovered over the
matching item in a list/tree view) now appear in ParaView's
user-preference settings dialog.

### Developer-facing changes

Going forward, SMTK should use ParaView's pattern for exposing
user-adjustable settings:

+ a server-side XML entry should exist in `smtk/extension/paraview/server/smconfig.xml` or
  another, similar XML config file. The XML entry must be in the "settings" ProxyGroup
  and also have an entry in a PropertyGroup that exposes it.
+ the property should be a member variable managed by a singleton in the
  same way that `vtkSMTKSettings::HighlightOnHover` is managed.
  If appropriate, place the member variable in vtkSMTKSettings;
  otherwise, create a new singleton.
+ any classes that wish to use the setting should then obtain the singleton
  holding the setting, fetch its value, and
    + immediately use the value (re-fetching each time just before it is used) or
    + observe changes to the setting via the Qt-VTK signal interop layer, e.g.,

```c++
pqCoreUtilities::connect(
  vtkSMTKSettings::GetInstance(), vtkCommand::ModifiedEvent,
  qtObjectPointer, SLOT(methodThatRespondsToSettingChange()));
```
