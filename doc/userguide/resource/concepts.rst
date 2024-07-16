Key Concepts
============

SMTK resources are much like documents in word processing or presentation software;
they are self-contained files that hold user data.
Documents in SMTK tend to contain information related to simulation preparation
such as geometric models, discretizations of these models, or information that
annotates geometric data.
Some documents may also contain user preferences and application state,
the same way a word processor saves the place you were last editing along
with the text of your correspondence.

There are three base classes holding provenance metadata:

:smtk:`PersistentObject <smtk::resource::PersistentObject>`
  serves as a base class for any object that will be serialized to/from
  file storage (and is thus persistent across runs of your SMTK application).
  Any such object must have a UUID and provide a name (the default
  implementation simply stringifies the UUID to produce a name, but this
  is not user friendly). Inheritance is used to specialize the type
  of information held by a persistent object (i.e., is it geometric data?
  does it conform to a schema? what methods provide access?).

:smtk:`Resource <smtk::resource::Resource>`
  instances correspond to files and thus have a URL.
  Each resource also has a UUID (since it inherits PersistentObject) and a type.
  Resources may also provide access to a resource of components (see below)
  present in the file, indexed by UUID.

:smtk:`Component <smtk::resource::Component>`
  instances are meaningful subsets of data present in a resource.
  An attribute resource presents its attributes as resource components.
  Similarly, geometric model entities (vertices, edges, faces, etc.) are
  components in the model subsystem.
  Each component has a UUID (since it inherits PersistentObject) and
  holds a weak reference to the resource which owns it.

In addition to these useful base classes,

:smtk:`Manager <smtk::resource::Manager>`
  instances hold a collection of resources.
  How the resources are related to each other is determined by your application,
  but typically there will be a single resource manager for the entire application.

:smtk:`Links <smtk::resource::Links>`
  Resource links connect persistent objects via a unidirectional
  relationship that exists for a specific purpose, or role.
  Links are discussed further below in the :ref:`resource links` section.

:smtk:`Properties <smtk::resource::Properties>`
  Resources and components contain a dictionary-like data structure that
  can hold any copyable type. If the property type has JSON bindings, it
  will be serialized to file along with the resource; otherwise, the
  property can still be stored, but will not be persistent. Currently
  enabled types include long, double, std::string, and std::vectors of
  these three types.
  As an example of how to extend the property system to handle
  non-POD (non-Plain Old Data) types, see the
  :smtk:`CoordinateFrame <smtk::resource::properties::CoordinateFrame>`
  property and its JSON serializer.

  Properties are stored separately from the objects they annotate;
  each resource instance owns the properties instance that holds all
  properties for both itself and its components. The
  :smtk:`smtk::resource::Properties` class is an abstract API provided
  on both resources and components; on the resource, the properties subclass
  provides actual storage, while on components, the subclass asks its
  parent resource for the properties object to search for values.
  Property data is a map from a string holding the data type-name
  (e.g., ``std::vector<int>``) to a subclass of :smtk:`TypeMapEntry <smtk::common::TypeMapEntry>`
  named :smtk:`PropertiesOfType <smtk::resource::detail::PropertiesOfType>`
  which owns a ``std::map<std::string, std::unordered_map<smtk::common::UUID, T>``
  (in our example, ``T`` is a ``std::vector<int>``, but in general ``T`` is any
  type allowed as a property value). Thus, properties map
  value-type-name → key-name → UUID → value.

:smtk:`Queries <smtk::resource::Queries>`
  Resources hold a container of relevant :smtk:`Query <smtk::resource::Query>` objects.
  Queries prevent the resource or component classes from growing large APIs
  and internal state by splitting methods that perform queries — such as
  identifying the spatial bounds of a mesh or model object, finding the closest point
  on a mesh or model component to some location in space, etc. — into separate
  classes that are easy to create and invoke.

  Furthermore, query objects can inherit their API from other queries, so it is
  possible to provide a uniform API with different implementations for each
  resource type.

  Queries may also need to store state in order to be performed efficiently.
  A good example is :smtk:`closest-point searches <smtk::geometry::ClosestPoint>`;
  usually many queries of this type are performed in a batch and a point locator
  structure is built to accelerate the query.
  The cache should outlive the query object, which is usually constructed on the
  fly by an algorithm, but also be marked dirty when the resource's components are
  modified. The Queries object owned by each resource provides a container for
  cache objects that individual Query objects may use. Multiple query classes can
  share the same cache object (e.g., ClosestPoint and ClosestCell might both use
  a PointLocator cache object).
