.. _release-notes-22.04:

=========================
SMTK 22.04 Release Notes
=========================

See also :ref:`release-notes-22.02` for previous changes.

SMTK Common Changes
===================================

Factory API for shared pointers
-------------------------------

The :smtk:`smtk::common::Factory` template now provides method
names that begin with ``make`` and which return shared pointers
(in addition to the ``create`` methods which return unique pointers).

This change was made to accommodate some compilers (e.g., gcc 11)
that are unable to cast unique pointers with an explicit deleter into
shared pointers.


SMTK QT Changes
=================

Enable QT_NO_KEYWORDS builds
----------------------------

SMTK compile definitions now include **QT_NO_KEYWORDS**, which means all uses of
`emit`, `foreach`, `signals`, and `slots` has been replaced with
`Q_EMIT`, `Q_FOREACH`, `Q_SIGNALS`, and `Q_SLOTS`, and must be updated for
any SMTK plugins that use the `smtk_add_plugin` cmake macro. This is to avoid
conflicts with 3rd-party libraries, like TBB, which use the old keywords.

Added names to qt UI objects to allow for more repeatable automatic testing
---------------------------------------------------------------------------

Automated tests were not giving repeatable results due to unpredicatble Qt
implicit naming of UI widgets.  To avoid this, all UI widgets were manually
assigned names using QObject::setObjectName()

Developer changes
~~~~~~~~~~~~~~~~~~

It would be good to use QObject::setObjectName() for any new Items to
avoid breaking this new "fully named" status.

User-facing changes
~~~~~~~~~~~~~~~~~~~

N/A


SMTK ParaView Extensions Changes
===================================

Added a Mechanism to hide labels for pqSMTKAttributeItemWidget
--------------------------------------------------------------

You can now suppress the item's label from being displayed by simply setting the label value to be " " (note a single blank and not the empty string).  As long as the item is not optional, its label will not be displayed.

3D Widget Visibility
--------------------------

3D widgets have an application-controlled setting that allows them to remain
visible even if their Qt controls loose focus or are hidden. This overrides
the ParaView default behavior of only allowing one 3D widget to be visible at
a time.

Developer changes
~~~~~~~~~~~~~~~~~~

By default, widget visibility behavior is not changed. To enable for an
application, call `pqSMTKAttributeItemWidget::setHideWidgetWhenInactive(false);`
on startup

User-facing changes
~~~~~~~~~~~~~~~~~~~

When this setting is enabled, multiple 3D widgets can appear in a renderview.
Widgets that might overlap need a user control to hide the widget, to allow
easier interaction. The last widget to be shown will receive precendence for
mouse and keyboard events.

SMTK Representation Changes
---------------------------

The ParaView representation now renders components with coordinate-frame
transforms applied.
This is implemented using a new :smtk:`vtkApplyTransforms` filter;
for resources without any transforms, this simplifies to an inexpensive
copy of the input blocks.
When transforms exists, VTK's ``vtkTransformFilter`` is used which can
be expensive for large geometry.
In the future, this cost may be avoided using a custom mapper.

Added the Ability to Render Geometry with Solid Cells
------------------------------------------------------

* vtkResourceMultiBlockSource will now put Components with Solid Cells under its component Component Block
* vtkApplyTransfors will now also do a surface extraction for components with solid cells


SMTK Project Changes
==========================

Made projects portable between different file systems and paths
---------------------------------------------------------------

Changed absolute file paths used in the project relative paths where
necessary to allow the project folder to moved to a new location, or
even another machine.

Developer changes
~~~~~~~~~~~~~~~~~~

No new absolute paths should be added to the project. Any new paths that
are saved should be saved as relative to the project main folder.  Ideally,
all files and folders that are part of a project should be contained
within the main project folder.

User-facing changes
~~~~~~~~~~~~~~~~~~~

With this change in plase, projects can be freely moved on a given machine,
or shared to a completely new machine in a seemless fashion.

SMTK Resource Changes
========================

Expanded SMTK Properties to include sets of ints
-------------------------------------------------

You can now create a property that is represented as a set of integers

Resource subclasses and the move constructor
--------------------------------------------

Because smtk's :smtk:`smtk::common::DerivedFrom` template, used when
inheriting Resource, declares a move constructor with the ``noexcept``
attribute, all subclasses of Resource must explicitly declare a
move constructor with the ``noexcept`` attribute. Modern versions of
clang are strict about checking this.

Improved ``smtk::resource::Metadata``
-------------------------------------

The :smtk:`smtk::resource::Metadata` class constructor now requires
create, read, and write functors which take an
:smtk:`smtk::common::Managers` instance as input so that creating,
reading, and writing resources can make use of any available
application-provided manager objects.

If you had any resource subclasses that provided these functors,
you must update to the new signature.
This is a breaking change.

Be aware that the operation and operation manager classes now accept
an :smtk:`smtk::common::Managers` instance.
If provided to the operation manager, all operations it creates will
have the managers object set (for use by operations).
This is the preferred way for applications to pass information to operations.
Using this method allows operations to be used in several applications
with minimal dependencies on application-specific methods and structures.

Finally, methods on the ``vtkSMSMTKWrapperProxy``, ``vtkSMTKWrapper``, and
``pqSMTKWrapper`` classes that returned a ``TypeContainer&`` have been
deprecated in favor of methods that return ``smtk::common::Managers::Ptr``
so that operations can make use of the type-container's contents without
copy-constructing a new Managers instance.

Coordinate Frame Property
-------------------------

SMTK's resource system now provides resources with a property
to store coordinate frames (an origin plus 3 orthonormal vectors
specifying a change of basis).
Besides the change of basis, each frame also holds an optional
UUID of a parent component.
If present and the parent also holds a matching coordinate frame,
the two transforms may be concatenated.

Because properties are named, coordinate frames may take on
different roles.
SMTK currently only deals with one role:
when a coordinate frame is stored under the name ``transform``
or ``smtk.geometry.transform``, it is taken to be a transformation
that should be applied to its associated component.
If the parent is unspecified, the coordinate frame transforms
the component's geometry into world coordinates.
If a parent is specified, then it transforms the component's
geometry into its parent's coordinates (which may or may not
be further transformed).

Future alternative uses for coordinate frames include

.. list-table:: Coordinate frame property names
   :widths: 15 30 5 50
   :header-rows: 1

   * - Short alternative
     - Formal alternative
     - 📦
     - Purpose
   * - ``transform``
     - ``smtk.geometry.transform``
     -
     - A coordinate frame relative to another component (or if none given, the world).
   * - ``centroid``
     - ``smtk.markup.centroid``
     -
     - The geometric center of a component.
       This may not always be identical to `pca` below if the component is defined as a surface discretization bounding a volume.
   * - ``center``
     - ``smtk.markup.center``
     -
     - The center of mass and principal axes (from computation of moments of inertia) that
       take a component's volumetric density field into account.
       Because density is involved, this is not always identical to `centroid`.
       It is in some cases, such as if the density is uniform.
   * - ``pca``
     - ``smtk.geometry.pca``
     -
     - The results of (p)rincipal (c)omponents (a)nalysis on the component's geometry.
       This is not always identical to `centroid` (when the component's geometry is a boundary representation)
       or `center` (when density is not uniform).
       PCA may also include "robust PCA," in which case it may warrant a distinct property name.
   * - ``landmark``
     - ``smtk.markup.landmark``
     - 📦
     - A point of interest that also has directions of interest.
   * - ``datum``
     - ``smtk.markup.datum``
     - 📦
     - A coordinate system used as a reference in analysis or geometry construction.
       This is distinct from landmarks, which are intrinsic features of geometry as opposed to
       datum frames used to construct geometry.
   * - ``obb``
     - ``smtk.geometry.obb``
     - 📦
     - An (o)riented (b)ounding (b)ox – non-unit-length vectors, but orthogonal.
       Axis lengths indicate bounds along each axis.
   * - ``screw``
     - ``smtk.geometry.screw``
     - 📦
     - Denavit–Hartenberg parameters for screw transforms.
       Either z-axis length indicates screw pitch (non-unit length) or
       an additional property or subclass would need be required for the screw pitch.
       Note that there is an alternate formulation of DH parameters
       called "modified DH parameters" which might deserve an additional distinct name.
   * - ``criticality``
     - ``smtk.markup.criticality``
     - 📦
     - The location and orientation of a vector-field criticality (such as a source,
       sink, or saddle point), perhaps with additional information in other properties
       such as the rotational components about each axis.
   * - ``symmetry``
     - ``smtk.geometry.symmetry``
     - 📦
     - A frame used to describe symmetric boundary conditions, solution periodicity, etc.
       Some extensions would be required to annotate more complex symmetries
       (cylindrical/spherical coordinate systems or n-way symmetry about an axis with n other than 2 or 4.

The table uses

* `smtk.geometry` for formal names of properties that might conceivably interact with the geometric data layer in SMTK (i.e., change what is rendered) and
* `smtk.markup` for formal names of properties that are annotations that might have illustrations but not affect the underlying geometric representation of the component itself.
* The "📦" column indicates whether the property would be used to hold a single coordinate frame or a collection of some sort (a fixed-size array, variable-length vector, set, map, etc.).


SMTK Graph Session Changes
==========================

Graph Arcs now support inverse relationships
--------------------------

Arcs may implement inverse relationships via the `Inverse` handler to seamlessly
add/remove arcs as coupled pairs.

Developer changes
~~~~~~~~~~~~~~~~~~

Arc(s) Required APIs:
 * insert(ToType&,bool)
 * erase(ToType&,bool)
 * begin()
 * end()


* Note, insert and erase take a second boolean argument that is used by the
  `Inverse` handler to break recursive insertion/removal of inverses.

The default behavior of operations for each type of arc type when inserted as the inverse are as follows.

.. list-table:: Default Arc Type Inter-op
   :widths: 10 10 30 30 20
   :header-rows: 1

   * - Self Arc Type
     - Inverse Arc Type
     - Assign
     - Insert
     - Erase
   * - Arc
     - Arc
     - Overwrite self, remove current inverse, insert new inverse.
     - Insert inverse if valid to insert self, insert self if inverse successfully inserted.
     - Unset self and erase inverse.
   * - Arc
     - Arcs
     - Overwrite self, remove current inverses, insert new inverse.
     - Insert inverse if valid to insert self, insert self if inverse successfully inserted.
     - Unset self and erase inverse.
   * - Arcs
     - Arc
     - Overwrite self, remove current inverses, insert new inverses.
     - Insert self, if successful insert inverse, if inverse failed remove self and report failure
     - Erase inverse and self.
   * - Arcs
     - Arcs
     - Overwrite self, remove current inverses, insert new inverses
     - Insert self, if successful insert inverse, if inverse failed remove self and report failure.
     - Erase inverse and self.
   * - OrderedArcs
     - Arc
     - Overwrite self, remove current inverses, insert new inverses.
     - Insert inverse, if successful insert self.
     - Erase inverse and self.
   * - OrderedArcs
     - Arcs
     - Overwrite self, remove current inverses, insert new inverses.
     - Insert inverse, if successful insert self.
     - Erase inverse and self.
   * - Arc or Arcs
     - OrderedArcs
     - Throw `std::logic_error` exception.
     - Throw `std::logic_error` exception.
     - Erase self and first matching inverse.
   * - OrderedArcs
     - OrderedArcs
     - Won't compile.
     - Won't compile.
     - Won't compile.

All behaviors can be overwritten by providing a specialization of
`smtk::graph::Inverse`.

User-facing changes
~~~~~~~~~~~~~~~~~~~

The user should not see any major changes. Management of arc types with an
inverse is now handled automatically so there may be some improvements to
arc consistency.

SMTK View Changes
=================

Fix Editing Descriptive Phrase Titles
-------------------------------------

When editing the title of a Descriptive Phrase of a Resource using the QT UI, the user is
presented with what was being displayed which included the Resource's name as well as with
Resource's location (in the case of the Resource Panel) or the Resource's Project role and name
(in the case of the Project Panel).

If the user didn't initially clear the title, the original title (including
the role or location information) was set as the Resource's new name instead of just the name component.

This has been corrected so that when editing the title, the user is only presented with the Resource's name.  When editing
is completed, the location/role information will be added properly.


Python Related Changes
======================

Make PyOperations execute on the main thread by default
-------------------------------------------------------

Python operations, including imported python operations, run on the
main gui thread by default, to avoid possible deadlocks because of
the Python global interpreter lock.

Python API changes
------------------

Due to fixes in recent Pybind11 releases, some types are no longer hashable in
Python. This requires changes in the Python API to change some return types
from Python ``set()`` instances into Python ``list()`` instances. This is
because ``std::set`` is, by default, turned into ``set()`` on the Python side,
but ordered on the C++ side does not imply hashable on the Python side. The
following APIs that have changed include:

  * ``smtk.mesh.MeshSet.cellFields()``
  * ``smtk.mesh.MeshSet.pointFields()``

The following types are no longer hashable:

  * ``smtk.mesh.CellField``
  * ``smtk.mesh.PointField``

Removal of Python2 support
--------------------------

Python2 support has been removed.



Plugin Changes
==============

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
