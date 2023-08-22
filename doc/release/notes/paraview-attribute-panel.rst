ParaView extensions
-------------------

Attribute editor panel fixes for the task system
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Previously, if your project contained an attribute resource whose display
hint prevented it from being shown in the attribute editor panel at load,
then a task which requested one of its views to be shown as the task was
activated would fail to display it. This happened because the panel assumed
the current attribute resource contained the view configuration for the
new view. This has been fixed by searching for the specified view name
using the application's resource manager.
