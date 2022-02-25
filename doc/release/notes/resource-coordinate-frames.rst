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
