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
