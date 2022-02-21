ParaView and Qt operation panels and views
------------------------------------------

The SMTK operation panel is now a "legacy" panel.
It is not deprecated, but it is not preferred.
Instead, consider using the operation toolbox and
parameter-editor panels.
By having two panels that split functionality from
the previous one, users with small displays do not
have to share limited vertical space among the operation
list and operation parameters.
There are other improvements below.

SMTK provides two new operation panels:
+ An "operation tool box" panel that shows a list of
  operation push-buttons in a grid layout (as opposed
  to the previous flat text list).
+ An "operation parameter editor" panel than contains
  a tabbed set of operations. By using tabs, multiple
  operations can be edited simultaneously.

Because most applications will want to choose between
either the legacy panel or the new panels, there are
now separate plugins:

+ The ``smtkPQLegacyOperationsPlugin`` exposes the legacy
  operations panel in applications.
+ The ``smtkPQOperationsPanelPlugin`` exposes the toolbox
  and parameter-editor operations panels in applications.

Your application can include or omit any combination of
these plugins as needed.

See the SMTK user's guide for more information about the panels.
