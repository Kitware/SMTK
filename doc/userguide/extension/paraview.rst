ParaView
========

SMTK provides ParaView plugins that, in concert with the VTK extensions, allow users to
load models and meshes in ParaView; perform selections and filtering on model entities;
and even run operations on models.
These plugins form the core of the ModelBuilder application (version 5 and later).

Some notes about the plugins:

* the server plugin/directory is for code that can be built
  without Qt and will reside — at least in part — on the server.
  Some proxy classes whose counterparts reside on the server are
  also included in this plugin.
* the representation plugin is dependent on the server plugin
  for the model (and eventually mesh) sources, for which it
  exposes representations.
  These representations display the *SMTK* selection, not
  the *ParaView* selection.
  The model representation is also useful because it uses
  a glyph mapper to draw instance prototype geometry at all
  instance placement points.
* the appcomponents plugin is dependent on the server plugin
  for the VTK-wrapped and CS-wrapped objects that it
  creates proxies for on the client.
  Many of the components in this plugin are ParaView "behaviors."
  A behavior is a QObject subclass that customizes the user interface of
  ParaView. In this case:
    * the :smtk:`pqSMTKResourceManagerBehavior` creates instances
      of :smtk:`smtk::resource::Manager` and :smtk:`smtk::resource::SelectionManager`
      on both the client and server and synchronizes them across the client-server connection.
    * the :smtk:`pqSMTKSelectionFilterBehavior` adds a toolbar to ParaView allowing users to
      specify what types of resource components they want to select.
      It then installs a filter onto an SMTK selection manager to force the selection to match
      the specification.
