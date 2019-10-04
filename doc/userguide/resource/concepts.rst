Key Concepts
============

There are two base classes holding provenance metadata:

:smtk:`Resource <smtk::resource::Resource>`
  instances correspond to files and thus have a URL.
  Each resource also has a UUID and a type.
  Resources may also provide access to a resource of components (see below)
  present in the file, indexed by UUID.

:smtk:`Component <smtk::resource::Component>`
  instances are meaningful subsets of data present in a resource.
  An attribute resource presents its attributes as resource components.
  Similarly, geometric model entities (vertices, edges, faces, etc.) are
  components in the model subsystem.
  Each component has a UUID and holds a weak reference to the resource
  which owns it.

In addition to these useful base classes,

:smtk:`Manager <smtk::resource::Manager>`
  instances hold a resource of resources.
  How the resources are related to each other is determined by your application,
  but a common use is holding all of the resources related to a simulation.

:smtk:`Links <smtk::resource::Links>`
Resource links connect persistent objects via a unidirectional
relationship that exists for a specific purpose, or role. Links are
stored separately from the objects they connect, but each resource
instance owns the link that holds all outgoing connections from the
resource and its components. The smtk::resource::Link class is an
abstract API provided on both resources and components; on the
resource, the link subclass provides actual storage, while on
components, the subclass asks its parent resource for the links object
in order to search for connections. Roles are simply integers. Roles
less than zero are reserved for internal SMTK use while positive roles
are available for user-specified (i.e., application-specific) roles.

:smtk:`Properties <smtk::resource::Properties>`
Resources and components contain a dictionary-like data structure that
can hold any copyable type. If the property type has JSON bindings, it
will be serialized to file along with the resource; otherwise, the
property can still be stored, but will not be persistent. Currently
enabled types include long, double, std::string, and std::vectors of
these three types.

Properties are stored separately from the objects they annotate, but
each resource instance owns the properties instance that holds all
properties for the resource and its components. The
smtk::resource::Properties class is an abstract API provided on both
resources and components; on the resource, the properties subclass
provides actual storage, while on components, the subclass asks its
parent resource for the properties object to search for values.
