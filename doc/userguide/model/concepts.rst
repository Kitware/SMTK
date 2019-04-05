Key Concepts
============

Like the attribute resource, the model system is composed of C++ classes,
also accessible in Python, whose instances perform the following functions:

:smtk:`Resource <smtk::model::Resource>`
  instances contain model topology and geometry.
  All of the model entities such as faces, edges, and vertices are
  assigned a UUID by SMTK.
  You can think of the resource as a key-value store from UUID values to
  model entities, their properties, their arrangement with other entities,
  their ties to the attribute resource, and their tessellations.

:smtk:`Session <smtk::model::Session>`
  instances relate entries in a model Resource to a solid modeling kernel.
  You can think of the entities in a model Resource as being "backed" by
  a solid modeling kernel; the session provides a way to synchronize
  the representations in the Resource and the modeling kernel.


:smtk:`Operator <smtk::model::Operator>`
  instances represent modeling operations that a modeling kernel
  provides for marking up, modifying, or even generating modeling entities
  from scratch.
  Operators usually require the entries in the model Resource to be
  updated after they are executed (in the solid modeling kernel).
  Each operator implements a method to invoke its operation in the modeling kernel
  and owns an attribute resource Attribute instance (its *specification*) to store
  the parameters required by the operation.
  SMTK expects the primary operand of an operator (e.g., a set of edge entities
  in the model resource to be swept into a face) to be model entities
  **associated** with the operator's specification.
  Other operands (e.g., the geometric path along which to sweep a set of edges)
  are stored as key-value Items in the specification.

:smtk:`EntityRef <smtk::model::EntityRef>`
  instances are lightweight references into a model Resource's storage
  that represent a single entity (e.g., a vertex, edge, face, or volume)
  and provide methods for easily accessing related entities, properties,
  tessellation data, and attributes associated with that entity.
  They also provide methods for manipulating the model Resource's storage
  but *these methods should not be used directly*; instead use an Operator
  instance to modify the model so that the modeling kernel and model resource
  stay in sync.
  EntityRef subclasses include Vertex, Edge, Face, Volume, Model,
  Group, UseEntity, Loop, Shell, and so on. These are discussed
  in detail in `Model Entities`_ below.

:smtk:`DescriptivePhrase <smtk::model::DescriptivePhrase>`
  instances provide a uniform way to present model entities and the information
  associated with those entities to users.
  There are several subclasses of this class that present an entity,
  a set of entities, an entity's property, and a set of entity properties.
  Each phrase may have 1 parent and multiple children;
  thus, phrases can be arranged into a tree structure.

:smtk:`SubphraseGenerator <smtk::model::SubphraseGenerator>`
  instances accept a DescriptivePhrase instance and enumerate its children.
  This functionality is separated from the DescriptivePhrase class so that
  different user-interface components can use the same set of phrases but
  arrange them in different ways.
  For example, a model-overview widget might subclass the subphrase generator
  to only enumerate sub-models and sub-groups of the entity in its input
  DescriptivePhrase; while a model-detail widget might include volumes, faces,
  edges, and vertices when passed a DescriptivePhrase for a model.


Model Entities
==============

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
