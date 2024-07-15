.. _resource links:

Resource Links
==============

Resources and components provide a facility, implemented via the
:smtk:`smtk::common::Links` template, to store relationships between
persistent objects. Each link is tagged with a *role* that indicates
the functional purpose of the relationship.

Links are stored separately from the objects they connect, but each
resource instance owns the link that holds all outgoing connections from the
resource and its components. The :smtk:`smtk::resource::Link` class is an
abstract API provided on both resources and components; on the
resource, the link subclass provides actual storage, while on
components, the subclass asks its parent resource for the links object
in order to search for connections. Roles are simply integers. Roles
less than zero are reserved for internal SMTK use while positive roles
are available for user-specified (i.e., application-specific) roles.

.. list-table:: SMTK system roles for Links
   :widths: 50 50
   :header-rows: 1

   * - Role
     - Purpose
   * - :smtk:`smtk::attribute::Resource::AssociationRole`
     - Link objects to an *instance* of an attribute to
       indicate they possess, exhibit, or manifest the attribute.
   * - :smtk:`smtk::attribute::Resource::ReferenceRole`
     - Link objects to an *item* of an attribute to indicate the
       item contains or references the object.
   * - :smtk:`smtk::mesh::Resource::ClassificationRole`
     - Link a mesh-set object to a source component whose geometry
       the mesh-set discretizes.
   * - :smtk:`smtk::resource::Resource::VisuallyLinkedRole`
     - Link objects to one another to indicate the source
       (left-hand) object (which does not have renderable geometry)
       should be provided with visibility controls for the target
       (right-hand) object(s) (which should have renderable geometry).

Persistent objects in the link system are referenced by their UUID.
This allows links to refer to resources and components not currently
loaded into an application (for instance, large meshes do not need
to be loaded to gather metadata on a simulation that uses the meshes
as domain discretizations).
The :smtk:`smtk::resource::Surrogate` class acts as a placeholder for
resources not currently loaded.

Links may have :smtk:`information <smtk::resource::LinkInformation>`  stored
with them (such as human-presentable names) so that resources not in memory
can still be displayed to users to provide context.

Links are stored in a two-level hierarchy of :smtk:`smtk::common::Links` instances:
the top-level Links instance holds connections between pairs of resources.
Resources connected at the top level always have a link role of ``smtk::resource::Links::TopLevel``.
Each top-level Link (singular) inherits a second Links (via the ``base_type`` template parameter)
instance holding all of the connections beteen resources and/or components.
In the bottom-level Links container, the user-provided roles are stored.
Resources are identified by null UUIDs for the ``left`` or ``right`` members
while components are identified by their UUID.
