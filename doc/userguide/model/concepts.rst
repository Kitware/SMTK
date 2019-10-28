Key Concepts
============

Like the attribute resource, the model resource is composed of C++ classes,
also accessible in Python, whose instances perform the following functions:

:smtk:`Resource <smtk::model::Resource>`
  instances contain model topology and geometry.
  All of the model entities such as faces, edges, and vertices are
  assigned a :smtk:`UUID <smtk::common::UUID>` by SMTK.
  You can think of the resource as a key-value store from UUID values to
  model entities, their properties, their arrangement with other entities,
  their ties to the attribute resource, and their tessellations.

:smtk:`Session <smtk::model::Session>`
  instances relate entries in a model Resource to a solid modeling kernel.
  You can think of the entities in a model Resource as being "backed" by
  a solid modeling kernel; the session provides a way to synchronize
  the representations SMTK keeps in a Resource and their native representation
  in the modeling kernel SMTK is interfacing with.

:smtk:`Entity <smtk::model::Entity>`
  is a subclass of :smtk:`Component <smtk::resource::Component>` specific to
  the model resource.
  The resource holds an Entity record for each vertex, edge, face, etc. since
  the types of information stored for geometric entities is similar.
  Methods that are specific to the type of geometry embodied by the entity
  are defined via the EntityRef class and its subclasses below.

:smtk:`EntityRef <smtk::model::EntityRef>`
  instances are lightweight references into the Entity records held by a
  model Resource's storage.
  Each represents a single entity (e.g., a vertex, edge, face, or volume)
  and provide methods for easily accessing related entities, properties,
  tessellation data, and attributes associated with that entity.
  They also provide methods for manipulating the model Resource's storage
  but *these methods should not be used directly*; instead use an Operation
  to modify the model so that the modeling kernel and model resource
  stay in sync.
  EntityRef subclasses include Vertex, Edge, Face, Volume, Model,
  Group, UseEntity, Loop, Shell, and so on. These are discussed
  in detail in `Model Entities`_ below.

Model Entities
==============

This section covers two aspects of SMTK's model entities:
the implementation details such as the organization of the class hierarchy;
and the principles and governing philosophy that the model entities embody.

Implementation
--------------

As mentioned above, the model :smtk:`Resource <smtk::model::Resource>` class
is the only place where model topology and geometry are stored in SMTK.
However, there are EntityRef-like classes, all derived from :smtk:`smtk::model::EntityRef`,
that provide easier access to model traversal.
These classes are organized like so:

.. findfigure:: entityref-classes-with-inheritance.*

   Each of the orange, green, purple, and red words is the name of an EntityRef subclass.
   The black arrows show relationships between instances of them (for which the
   classes at both ends provide accessors).

Each relationship shown in the figure above has a corresponding
method in the EntityRef subclasses for accessing the related entities.

Organizing Principles
---------------------

Model components are geometric entities;
they represent geometric features that simulations use to specify the
problem domain and geometric constraints on the problem due to physical
phenomena, natural conditions, engineered behavior, and variations over time.
Most of the entities that SMTK provides are related to boundary-representation_
(B-Rep) models, where a domain is specified by its boundary (and the boundary
entities are specified by their boundaries, decreasing in dimension to
vertices). Thus B-Rep models specify volumes via bounding faces, faces via
bounding edges, and edges by vertices. Not every entity necessarily has a
lower-dimensional boundary: an edge may be periodic and thus not need a
vertex as its boundary, or infinite in extent (although usually, those are
represented by a vertex placed "at infinity" via homogeneous coordinates).
Similarly, faces and volumes may be unbounded due to periodicity or
infinite extent.

The other common feature in most B-Rep models are *use-records*,
which impart sense and orientation to geometric entities.
Geometric entities such as faces are generally considered either
topologically (i.e., an entity is a single thing (a set) given a name
inside a collection of other things) or geometrically (i.e., an entity
is a locus of points in space, and those points have features such as
a measure that identifies the dimensionality of the locus).
Neither of these approaches (topological or geometric) imply
*orientation* or *sense*.

Orientation is a binary indicator (positive/negative, inside/outside, or
forward/backward) associated with an entity.
Occasionally, people may also consider it a tertiary indicator:
inside/outside/on.

Similarly, the "sense" of an entity is the notion of how the entity
is being employed to compose the model.

To understand how sense and orientation are distinct from one another,
consider an edge in a 3-dimensional model that bounds 2 or more faces.
Edges have both an orientation, which is the direction along the edge
considered forward.
The sense identifies which face a pair of oppositely-oriented "uses" bound.
Since the same edge may bound arbitrarily-many faces in 3 dimensions,
SMTK uses an integer to identify the sense.

Faces (in 3-dimensional models) always have 2 orientations and
may partition volumes to either side.
Because a face only has 2 sides, faces may bound at most two volume regions.
This means that for faces, the sense may always be taken to be 0 without
loss of generality.

Vertices are not considered to have an orientation but do have a
non-trivial sense: consider the model vertex at the point where two conical surfaces meet.
The vertex is used in a different sense by each cone; a different vertex-use record
(if vertex uses are tracked by the modeling kernel) will be created for each
volume bounded by the vertex when those volumes share no other points
in the immediate neighborhood of the vertex.

Beyond geometric entities and use-records, SMTK also offers model entities
for less formal models: groups, auxiliary geometry, geometric instances,
models, and concepts.

.. _boundary-representation: https://en.wikipedia.org/wiki/Boundary_representation

Filtering and Searching
=======================

As with all classes that inherit :smtk:`smtk::resource::Resource`, it is possible
to ask the resource to filter its components using a string that specifies some
search criteria (i.e., a filter).
Model resources accept an extended syntax compared to other resource types
since (1) model entities have so many types as described above
and (2) in addition to these types, users and SMTK workflows often mark up these
model entities with properties (covered in the :ref:`model-properties` section)
to provide high-level conceptual information that is useful in preparing simulations.
For example, a geometric model of a motor will have many model faces that might
each be marked with properties to indicate which are bearing surfaces, which are
fastener or alignment surfaces, which surfaces will be in contact with coolant
or fuel, etc.

In order to allow user interface components to only show relevant model entities,
the model resource's :smtk:`queryOperation <smtk::model::Resource::queryOperation>`
method accepts strings in the following format:

    type-specifier ``[`` property-type  [ ``{`` property-name [ ``=`` property-value ] ``}`` ]

where

+ ``type-specifier`` is any model-entity type specifier string such as `face`, `group`, `model`.
  A full list can be found in ``smtk/model/Entity.cxx``.
+ ``property-type`` is one of the following string literals ``string``, ``floating-point``, ``integer``.
+ ``property-name`` is either a single-quoted name or a slash-quoted regular expression
  (i.e., a regular expression surrounded by forward slashes such as ``/(foo|bar)/)``.
+ ``property-value`` is one of the following
    + a single, single-quoted string value to match
      (when searching for string properties),
    + a single, slash-quoted regular expression to match
      (when searching for string properties by regular expression),
    + a single, unquoted integer or floating point value to match
      (when searching for properties of those types), or
    + a tuple (indicated with parentheses) of values, as specified above,
      to match. Note that this implies the property must be vector-valued
      and the length must match the specified tuple's length in order
      for a match to be successful.

Whitespace is allowed anywhere but is treated as significant if it is inside
any quoted string value or regular expression.

Note that single quotes are used because these filter strings
will appear in XML and/or JSON serializations that use double-quotes
to mark the start and end of the query string.
The examples below include the double-quotes around the query as a reminder.

For regular expressions, the c++11 standard library is used to search for matches;
the syntax must be accepted by the std::regex constructor and std::regex_search()
must return true when passed property names or values in order for the
corresponding entity to be included in filtered results.

.. list-table:: Examples of valid query strings.
    :widths: 40 80
    :header-rows: 1

    * - Query string
      - Results
    * - "``model|3``"
      - Any model explicitly marked as 3-dimensional. (This example
        has no limiting clause is here to be clear that existing query
        strings will continue to be accepted.)
    * - "``vertex[string]``"
      - Vertices with any string properties at all (but not vertices without string properties).
    * - "``any[integer{'counter'}]``"
      - Any entity with an integer property named 'counter' (regardless of the value).
    * - "``face[string{'pedigree'='zz'}]``"
      - Faces with a string-property named pedigree whose value is "zz"
    * - "``any[floating-point{/.*/=(0,0,0)}]``"
      - An entity of any type with any floating-point property whose value is a 3-entry vector of zeros.
    * - "``group[string{'alphabet'=('abc', 'def')}]``"
      - Any group with a string property named "alphabet" whose value is a vector of 2 strings: one valued "abc" and the next valued "def".

.. list-table:: Invalid non-examples of query strings that will not work.
    :widths: 40 80
    :header-rows: 1

    * - Query string
      - Why This is Invalid
    * - "``edge,face[integer{'pedigree'=23}]``"
      - Multiple queries are not supported yet.
        Also, it is unclear whether the limiting clause applies
        to both types or just faces.
        For now, use multiple filters to handle combination queries
        with different limiting clauses.
        Note that if this example had used ``edge|face`` instead of ``edge,face``,
        it would have been valid; the filter would have apply to edges or faces.
    * - "``any[{'pedigree'}]``"
      - You must currently specify the property type.
    * - "``any[integer{'lattice'=(0,*,*)'}]``"
      - There is no way to search for properties with partially-matched array-valued entries.
    * - "``any[integer{'counter'=(*,*,*)'}]``"
      - There is no way to search for properties whose value is a given length yet.
