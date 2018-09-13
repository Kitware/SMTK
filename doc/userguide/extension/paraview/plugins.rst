Plugins
-------

Some notes about the plugins:

* the server directory (which holds source for the smtkPVServerExtPlugin plugin) is
  for code that can be built without Qt and will reside — at least in part — on the server.
  Some client-side proxy classes (that do not use Qt) whose counterparts
  reside on the server are also included in this plugin.
  This is where the model (and eventually mesh) sources and their representations reside.
  Note that the representations display the *SMTK* selection, not the *ParaView* selection,
  although the two selections are generally kept in sync.
  Besides dealing with SMTK selections in a consistent way,
  the model representation uses a glyph mapper to draw instance prototype geometry at all
  instance placement points.
* the appcomponents plugin is dependent on the server plugin
  for the VTK-wrapped and CS-wrapped objects that it
  creates proxies for on the client.
  Many of the components in this plugin are ParaView "behaviors."
  A behavior is a QObject subclass that customizes the user interface of
  ParaView. In this case:
    * the :smtk:`pqSMTKBehavior` creates instances of :smtk:`vtkSMTKWrapper` objects
      on the server and manages them via :smtk:`vtkSMSMTKWrapperProxy` objects on the client.
      Each wrapper exposes an :smtk:`smtk::resource::Manager`, an :smtk:`smtk::operation::Manager`,
      and an :smtk:`smtk::view::Selection` to server-side VTK classes (such as the resource
      readers and representations).
    * the :smtk:`pqSMTKSelectionFilterBehavior` adds a toolbar to ParaView allowing users to
      specify what types of resource components they want to select.
      It then installs a filter onto an SMTK selection manager to force the selection to match
      the specification.
    * the :smtk:`pqSMTKResourcePanel` class adds a panel to ParaView that shows the resources
      and components available.
    * the :smtk:`pqSMTKColorByToolBar` class adds a tool bar to ParaView that allows users
      to choose how model entities should be colored (e.g., by their assigned color, by the
      assigned color of the volume they bound, by field values such as simulation results).
