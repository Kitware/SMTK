Key Concepts
============

A major role of model and mesh resources is visualization and
interactive manipulation of a simulation's domain.
Additionally, some SMTK sessions may wish to perform
analytical modeling tasks that require access to geometric data,
such as approximating integrals over subsets of the simulation domain.
These tasks require a geometric description of resources and components.
While SMTK's core library does not provide direct support for
geometry — since that would introduce unwanted dependencies
for many use cases — it does include support (if plugins providing
geometry are loaded) for querying what components and resources have
geometric data and for querying the bounds of any related geometry.

Furthermore, it is possible for a resource to provide
support for multiple backends that represent geometry
differently (e.g., VTK, VTK-m, vtk-js, Unity, Vulkan, PDF).
It is even possible that multiple backends might be used
simultaneously; for example, an application might provide
interaction using VTK but use a different backend like vtk-js
to publish content to a cloud service.

Another constraint on geometry providers is the need for
caching of geometry in order to efficiently respond to
repeated queries for bounding boxes, picking, etc.
Many times, these operations require expensive tessellation
operations and the result should be cached for re-use.
Thus the interface that provides geometry should allow for
a significant amount of state to be held; expecting each
query for a bounding box to construct a new instance of
some geometry provider would complicate efficient caching
for repeated queries.

Therefore, SMTK provides:

:smtk:`smtk::geometry::Backend`
  which is a mostly-empty class to be inherited by actual rendering backends.
  It is used as a key so that each resource can own multiple geometry
  provider objects as needed, while providing type-safe access to
  specific subclasses needed by different renderers.
  Currently, only :smtk:`smtk::extension::vtk::source::Backend` inherits it.

:smtk:`smtk::geometry::Geometry`
  which is a base class for objects that can respond to geometric queries.
  This class does not provide methods that return any actual geometric data;
  subclasses must implement methods for querying the existence of
  renderable geometry and its bounds for all the persistent objects owned
  by the parent resource of the geometry object.
  Thus, metadata such as existence and bounds are available to libraries
  linked only to SMTK's core library.

:smtk:`smtk::geometry::GeometryForBackend`
  which all concrete implementations should inherit since it can respond
  to geometric queries in a specific format (its template parameter).
  Backends will use the pure virtual data() method it declares to return
  geometry in a format specific to the backend.

:smtk:`smtk::geometry::Cache`
  which is a convenience class for writing Geometry subclasses that will
  maintain a cache of geometric data for each object.
  Concrete implementations of
  of :smtk:`GeometryForBackend<Format> <smtk::geometry::GeometryForBackend>`
  may choose to inherit this class, which implements several virtual methods
  declared in Geometry and defines new methods you must implement
  in order to populate the cache.

:smtk:`smtk::geometry::Generator`
  is a subclass of :smtk:`smtk::common::Generator` used to register
  and create instances of Geometry subclasses on demand.
  Note that generators return unique pointers (i.e., std::unique_ptr<Geometry>)
  so that large caches are not duplicated or multiply-owned.
  This class is used internally by :smtk:`smtk::geometry::Resource`


.. findfigure:: geometry.*

   This figure shows how rendering code is split across 3 libraries
   (left: the core, which defines a basic interface with no external dependencies),
   a rendering extension (right: a plugin used by the application to allow
   geometry interaction), and a session (center: a plugin loaded by the application
   to provide a new type of model, mesh, or attribute resource that has geometric
   components).

Writing a geometry object for a resource
----------------------------------------

When you create a new resource type with renderable
geometry, you must (1) create new geometry class(es)
that subclass GeometryForBackend<Format> (one for each
Backend you wish to support), and (2) register
the geometry class(es) so they will be created when needed.
We'll cover the latter task (registration) now and come
back to subclassing a geometry provider afterward.

Each geometric Resource contains a map from geometry backend IDs to Geometry instances.
When asked for geometry that it does not already have an instance for,
the resource uses a :smtk:`Generator <smtk::geometry::Generator>` to query
the application for one.
There is no guarantee that geometry will exist;
this may be because a particular backend is not supported or because
the application has not been linked with a library that supports the
backend.

For example, consider a lightweight, 2-D model resource that is used
extensively by python scripts to do batch processing.
You would probably not want the python module that registers this model resource
to link to VTK since that could increase script runtimes adversely and the scripts
do not need to provide renderings of your model.
However, you probably want the modelbuilder application to be able to render
components of your model.
For this reason, you should put the VTK-based geometry in a separate
library from the resource's core implementation.
The library with VTK dependencies can then expose a geometry object
by implementing

.. code-block:: c++

    class RegisterResourceFooBackendBar
      : public smtk::geometry::Supplier<
          RegisterResourceFooBackendBar>
    {
    public:
      bool valid(const geometry::Specification& in) const override
      {
        // Only create providers for the "bar" backend:
        smtk::bar::Backend backend;
        return std::get<1>(in).index() == backend.index();
      }

      smtk::geometry::GeometryPtr operator()
        (const geometry::Specification& in) override
      {
        // Only create providers for "foo" resources:
        auto rsrc = std::dynamic_pointer_cast<
          smtk::foo::Resource>(std::get<0>(in));
        if (rsrc)
        {
          auto geom = new foo::BarGeometry(rsrc);
          return smtk::geometry::GeometryPtr(geom);
        }
        return nullptr;
      }
    };

Whenever an application needs to render a resource,
it asks the resource for a geometry object compatible
with a given backend (such as VTK).
If none already exists, a geometry::Generator
is used to iterate over registered geometry::Supplier
instances like the one above until a match is found or
the registered list is exhausted.

Consuming geometry from a resource
----------------------------------

Subclasses of `smtk::geometry::Backend`,
should provide a method on the backend which accepts a geometry provider and
a persistent object and returns its corresponding geometry.
Internally, this method can cast the geometry provider it is passed
into one or more `smtk::geometry::GeometryForBackend<T>` subclasses.
When a match is identified, that provider's geometry() method can be called
to fetch geometry and convert it into something the rendering backend can use.

In this way, it is possible for one backend to use providers originally written
for different backends. As an example, a VTK-m backend might include an adaptor
for dealing with providers that can supply VTK data objects.
