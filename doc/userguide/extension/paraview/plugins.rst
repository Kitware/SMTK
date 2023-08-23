Plugins
-------

Some notes about the plugins:

* the server directory (which holds source for the smtkPVServerExtPlugin plugin) is
  for code that can be built without Qt and will reside — at least in part — on the server.
  Some client-side proxy classes (that do not use Qt) whose counterparts
  reside on the server are also included in this plugin.
  This is where the resource representations reside.
  Note that the representations display the *SMTK* selection, not the *ParaView* selection,
  although the two selections are generally kept in sync.
  Besides dealing with SMTK selections in a consistent way,
  the model representation uses a glyph mapper to draw instance prototype geometry at all
  instance placement points.
* the appcomponents plugins are dependent on the server plugin
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
  Besides behaviors, other user-interface
  components include:
    * the :smtk:`pqSMTKResourcePanel` class adds a panel to ParaView that shows the resources
      and components available.
    * the :smtk:`pqSMTKColorByToolBar` class adds a tool bar to ParaView that allows users
      to choose how model entities should be colored (e.g., by their assigned color, by the
      assigned color of the volume they bound, by field values such as simulation results).
    * the :smtk:`pqSMTKSubtractUI` class is a singleton that provides methods for
      removing ParaView UI components (and restoring them if needed).
      Your plugin may call methods on it to customize the application for
      a particular workflow.
      Note that many user interface components can only be removed after the event
      loop has started; if you want to disable components from within a plugin's
      start up, you should use a QTimer to schedule calls to the UI subtractor
      once the event loop has started.
      Dock-widget panels in particular require approximately a ~1 second delay in order
      to be removed at application startup.
    * the :smtk:`ApplicationConfiguration <smtk::paraview::ApplicationConfiguration>` class
      defines a pure virtual interface that applications implement and expose in an
      application-specific plugin in order to configure SMTK's user-interface components.
      This is currently only used by the operation-toolbox panel but is likely to expand to
      other components.
* some extension directories create multiple paraview plugins.
  This is done because the SMTK library containing the components
  has a well-defined purpose, but not every application using SMTK
  wishes to expose all of the components in the library.
  Therefore, several ``plugin-`` subdirectories may exist and each
  one exposes a different set of components from the library.
  The appcomponents directory is an example of this:
    * The ``plugin-core`` directory exposes the majority of user interface components;
    * The ``plugin-legacy-operations`` directory exposes a panel for debugging operations;
    * The ``plugin-operations-panel`` directory exposes two production-ready panels for
      choosing operations and editing their parameters; and
    * The ``plugin-panel-defaults`` directory exposes an application-configuration interface class.

Python operation plugins
~~~~~~~~~~~~~~~~~~~~~~~~

ParaView's plugin manager allows you to add python files as plugins.
These files are then imported when ParaView (and thus modelbuilder)
are initialized.
In order to register a python operation (let's say it is named ``feature_op``),
just place the following at the bottom of the python file you add to ParaView
as a plugin.

.. code:: python

   import smtk.operation
   class feature_op(smtk.operation.Operation):
       def __init__(self):
           smtk.operation.Operation.__init__(self)
       # …

   if __name__ != '__main__':
       try:
           import smtk.extension.paraview.appcomponents as pv
           pv.importPythonOperation(__name__, 'feature_op')
       finally:
           pass

You can call ``importPythonOperation()`` as many times as you like (to
register many python operations).
