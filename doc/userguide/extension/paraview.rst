ParaView
========

SMTK provides ParaView plugins that, in concert with the VTK extensions, allow users to
load models and meshes in ParaView; perform selections and filtering on model entities;
and even run operations on models.
These plugins form the core of the ModelBuilder application (version 5 and later).

In ParaView, there are 3 types of objects that SMTK frequently interacts with:

* Subclasses of vtkObject which live on each server process and are "wrapped"
  by C++ code that allows remote invocation of their methods across the
  client-server (CS) connection each server maintains with either the rank-0
  server or the client.
  Several SMTK classes have instances owned by a "wrapper" class that inherits
  from vtkObject to make this communication possible.
* Subclasses of vtkSMProxy, which live on the client process.
  Subclasses specific to SMTK are only required when the client needs to expose
  methods to manage SMTK data on the server that cannot be handled by
  ParaView's client-server wrapping. For example, methods that take non-primitive
  objects, such as pointers to SMTK classes as input, since these methods
  cannot be CS-wrapped.
* Subclasses of pqProxy, which live on the client process and reference
  an instance of vtkSMKProxy (or its subclasses).
  Subclasses specific to SMTK exist to expose Qt signals and slots related to
  SMTK. If Qt is not required, then you should subclass vtkSMProxy instead.

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

Lifecycle
---------

Below are some examples of the way objects are managed in ParaView-based SMTK applications.
These examples help describe the communication patterns in ParaView, which are constrained
by the assumption that data is distributed in parallel on 1 or more server processes.
One unintuitive implication of this assumption is that almost all communication begins on
the client and moves to the server(s).
This makes some things difficult since events on the server cannot result in client-side actions.

1. Plugin initialization.
   SMTK registers, via CMake, an autostart plugin.
   This plugin's startup method creates an instance of :smtk:`pqSMTKBehavior`
   which listens for ParaView servers attaching to/detaching from the client.
   When one attaches, it directs ParaView's pqObjectBuilder to create an instance of
   :smtk:`vtkSMTKWrapper` on the server and :smtk:`vtkSMSMTKWrapperProxy` on the client.
   The plugin also registers (again via CMake macros) a pqProxy subclass named
   :smtk:`pqSMTKWrapper` to be created whenever a vtkSMSMTKWrapperProxy
   is instantiated.
2. Reading an SMTK file.
   SMTK registers, via an XML configuration file, a VTK-based reader (:smtk:`vtkSMTKModelReader`)
   for the files which outputs a multi-block dataset.
   ParaView's file-open dialog then constructs instances of the reader on the server as well
   as proxies (:smtk:`pqSMTKResource`) on the client.
   The client proxies connect to the resource manager and register the resource with the
   SMTK resource manager instances owned by the client and server. (At this point, the
   client does not own a separate instance but instead uses the server's.)
3. Displaying an SMTK model.
   When an SMTK model resource is read (as described above), ParaView creates
   vtkPVDataRepresentation objects (on the servers) and a
   vtkSMRepresentationProxy and a pqRepresentation instance on the client for each view.
   Instead of creating a pqRepresentation, SMTK's XML configuration tells ParaView to
   create a subclass named :smtk:`pqSMTKModelRepresentation`.
   Similarly, on the server, :smtk:`vtkSMTKModelRepresentation` instances are created
   instead of vtkPVDataRepresentation instances.
   The vtkSMTKModelRepresentation instance looks for an instance of
   vtkSMTKResourceManagerWrapper. If one exists, then it uses the SMTK selection
   owned by the resource manager when rendering.
