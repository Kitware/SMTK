ParaView Plugins
----------------

SMTK's ParaView ``appcomponents`` plugin has been split into 2 plugins and 2 new plugins have been added.
Where previously the ``smtkPQComponentsPlugin`` held all functionality,
the operation panel has been moved into ``smtkPQLegacyOperationsPlugin`` so that
applications based on SMTK may exclude it.
Likewise, the new operation toolbox and parameter-editor panels are in a new ``smtkPQOperationsPanelPlugin``.
Applications may include any combination of the operation-panel plugins.

Finally, a new ``ApplicationConfiguration`` interface-class has been provided.
The toolbox panel waits to configure itself until an subclass instance is registered
with ParaView, at which point it is queried for a view-information object.
Use of this pattern is likely to expand in the future as panels add more flexibility
to their configuration.
