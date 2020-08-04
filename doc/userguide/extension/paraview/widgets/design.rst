Design
------

Because SMTK is using ParaView, which is an application built on top of VTK,
designing new 3-d widgets for use with SMTK involves several API boundaries
and thus several classes.

VTK.
  The basic concept of a 3-d widget is implemented in VTK without any
  notion of a matching Qt user interface.
  VTK provides 2 classes that new widgets should inherit: a representation class
  which is responsible for generating renderable data to provide the visual
  representation of the widget and
  a widget class, which is responsible for user interaction related
  to the visual appearance (i.e., translating mouse hover, click, drag, and
  even keyboard events into parameter changes that affect the visual representation).
  For some widgets, a third class is used to hold parameters that define an
  implicit function which can be used to model related geometry.

  A good example of these classes are :smtk:`vtkConeRepresentation` and :smtk:`vtkConeWidget`.
  The implicit function representing the cone being modeled is
  the :smtk:`vtkImplicitConeFrustum` class.
  Frequently, these implicit functions are used to select cells, trim geometry,
  or perform other editing actions as the user interacts with the widget.

ParaView.
  ParaView uses VTK 3-d widgets. These widgets are rendered on the server
  but must respond to user interaction that takes place on a desktop client.
  ParaView uses its client-server protocol to distribute mouse and keyboard events
  to the server and its client-server property system to retrieve updated parameters
  from the server as needed (e.g., a sphere's center and radius might be updated
  by a VTK widget+representation on the server and then transferred to the client
  via two DoubleVector properties).
  ParaView's widgets have both a 3-d appearance (obtained via VTK) and a 2-d Qt user
  interface that allows users to inspect and enter exact numeric values when needed.
  ParaView widgets inherit the pqInteractivePropertyWidget class.

  A good example of a ParaView widget class is :smtk:`pqConePropertyWidget`, which
  uses the VTK classes above, but indirectly via server-manager XML in the
  plugin named smtkPVServerExtPlugin; when a pqConePropertyWidget is created on
  the client process, it constructs a proxy object named ConeWidgetRepresentation.
  This proxy object has subproxies named ConeWidget and ConeRepresentation that
  are backed on the server process by the VTK classes above.
  When users interact with the VTK classes in the render window, changes to those
  classes are mapped to ParaView server-manager proxy property objects and
  notifications are sent to the client that properties are updated.
  Similarly, changes on the client change proxy properties that are transmitted
  to the server and used to update the VTK classes.

SMTK.
  SMTK maps ParaView properties back and forth between SMTK attribute items.
  For example, the cone widget in ParaView requires properties for a generic
  truncated cone (two points — to determine the axis — and two radii —
  to determine the radius perpendicular to that axis at each endpoint).
  However, an SMTK attribute may only want to accept cylinders (a single
  radius for each point). The SMTK qtItem subclass maps between the items
  available in the attribute and the properties in ParaView.
  Each time a ParaView property is set, the updateItemFromWidgetInternal method
  is invoked. Likewise, when the attribute is modified externally (for example,
  by a Signal operation being performed), the updateWidgetFromItemInternal
  method is invoked.
  These methods both generally invoke a protected method specific to each
  widget to fetch items from the attribute.
  Based on the items available, these two methods synchronize values between
  SMTK attribute items and ParaView properties.

  A good example of an SMTK item widget is :smtk:`pqSMTKConeItemWidget`.
