.. _model-properties:

Model Property System
=====================

In addition to associating modeling entities with attributes,
SMTK's model manager can also store string, integer, and floating-point
properties on model entities.
Unlike attributes that have a rigid format imposed by definitions,
model properties are free-form: given any model entity UUID and a
property name, you may store any combination of string, integer, and
floating-point values.

The intent of the model property system is two-fold:

1. Properties can be created by bridges to expose modeling-kernel
   data that cannot otherwise be expressed by SMTK.
   An example of this is the Exodus bridge, which stores the
   Exodus ID of each block and set it reads so that exporters
   can refer to them as they exist in the file.

2. Properties can be created by applications built on SMTK to
   mark up model entities.
   This might include user markup such as entity names
   or application data such as whether an entity is currently
   selected, or which views an entity should be displayed in.

Despite being free-form, SMTK does use the property system itself
and so some property names are reserved for particular use.
The list of these names is below and is followed by properties
provided by existing bridges.

.. _reserved-model-properties:

Reserved model properties
-------------------------

The following table lists the conventions that SMTK uses for property names.
These names are used for the purpose described but the convention is not
enforced programmatically.

Properties that applications will most commonly expose to users are summarized
in the table below:

.. NOTE: Keep these alphabetical and use the "Blank row" below for convenience. Do not make the table any wider than it is!

+--------------------------------+---------------+----------------------------------------------------------------------------+
| Property name                  | Property type | Description                                                                |
+================================+===============+============================================================================+
| color                          | Float         | A 3- or 4-tuple holding RGB or RGBA color values. Clamped to [0,1].        |
|                                |               | An example would be [0., 0., 1., 1.] for blue.                             |
|                                |               | Color may be assigned to any entity with a visual representation.          |
+--------------------------------+---------------+----------------------------------------------------------------------------+
| embedding dimension            | Integer       | The number of coordinates required to represent any point in the locus     |
|                                |               | of all points contained in the entity's underlying space.                  |
+--------------------------------+---------------+----------------------------------------------------------------------------+
| name                           | String        | The name for the entity as presented to the user.                          |
+--------------------------------+---------------+----------------------------------------------------------------------------+
| url                            | String        | Assigned to model entities loaded from data at the specified location.     |
+--------------------------------+---------------+----------------------------------------------------------------------------+

Properties used internally are in the following table:

.. tabularcolumns:: l|l|p{4in}

+--------------------------------+---------------+----------------------------------------------------------------------------+
| Property name                  | Property type | Description                                                                |
+================================+===============+============================================================================+
| cell_counters                  | Integer       | An array of 6 integers assigned to each model entity and                   |
|                                |               | used to generate model-unique names that are easier to read than UUIDs.    |
|                                |               |                                                                            |
|                                |               | See :smtk:`Entity::defaultNameFromCounters()` to understand how the array  |
|                                |               | is used.                                                                   |
+--------------------------------+---------------+----------------------------------------------------------------------------+
| generate normals               | Integer       | When non-zero, this indicates that the given entity's tessellation should  |
|                                |               | have surface normals added via approximation (because the modeling kernel  |
|                                |               | does not provide any and the geometry has curvature).                      |
|                                |               | When not defined, it is assumed to be zero. When defined, it should be a   |
|                                |               | single value.                                                              |
|                                |               |                                                                            |
|                                |               | Most commonly this is stored on models and is applied to all of the        |
|                                |               | tessellated entities belonging to the model. However, it may be stored     |
|                                |               | on individual entities as well.                                            |
+--------------------------------+---------------+----------------------------------------------------------------------------+
| group_counters                 | Integer       | An array of 3 integers assigned to each model entity and                   |
|                                |               | used to generate model-unique names that are easier to read than UUIDs.    |
|                                |               | Three integers are used because groups with the :smtk:`MODEL_DOMAIN` or    |
|                                |               | :smtk:`MODEL_BOUNDARY` bits set are numbered separately.                   |
|                                |               |                                                                            |
|                                |               | See :smtk:`Entity::defaultNameFromCounters()` to understand how the array  |
|                                |               | is used.                                                                   |
+--------------------------------+---------------+----------------------------------------------------------------------------+
| instance_counters              | Integer       | A single integer assigned to each model entity and used to generate        |
|                                |               | model-unique instance names that are easier to read than UUIDs.            |
|                                |               |                                                                            |
|                                |               | See :smtk:`Entity::defaultNameFromCounters()` to understand how the        |
|                                |               | counter is used.                                                           |
+--------------------------------+---------------+----------------------------------------------------------------------------+
| invalid_counters               | Integer       | A single integer assigned to each model entity and used to number invalid  |
|                                |               | child entities in a way that is easier to read than UUIDs.                 |
|                                |               |                                                                            |
|                                |               | See :smtk:`Entity::defaultNameFromCounters()` to understand how the        |
|                                |               | counter is used.                                                           |
+--------------------------------+---------------+----------------------------------------------------------------------------+
| model_counters                 | Integer       | A single integer assigned to each model entity and used to generate        |
|                                |               | a unique name for each submodel of the given model.                        |
|                                |               |                                                                            |
|                                |               | See :smtk:`Entity::defaultNameFromCounters()` to understand how the        |
|                                |               | counter is used.                                                           |
+--------------------------------+---------------+----------------------------------------------------------------------------+
| bridge pedigree                | String or     | A bridge-specific persistent identifier assigned to the associated entity  |
|                                | Integer       | for use by the exporter and other tasks that need to refer to the entity   |
|                                |               | when it is not possible to use UUIDs created by SMTK to do so.             |
|                                |               | This happens when the original model file may not be modified and          |
|                                |               | simulation input decks must refer to entities in that original file.       |
|                                |               |                                                                            |
|                                |               | Bridges should provide 0 or 1 values for each entity.                      |
+--------------------------------+---------------+----------------------------------------------------------------------------+
| shell_counters                 | Integer       | An array of 5 integers assigned to each model entity and                   |
|                                |               | used to generate model-unique names that are easier to read than UUIDs.    |
|                                |               |                                                                            |
|                                |               | See :smtk:`Entity::defaultNameFromCounters()` to understand how the array  |
|                                |               | is used.                                                                   |
+--------------------------------+---------------+----------------------------------------------------------------------------+
| use_counters                   | Integer       | An array of 6 integers assigned to each model entity and                   |
|                                |               | used to generate model-unique names that are easier to read than UUIDs.    |
|                                |               |                                                                            |
|                                |               | See :smtk:`Entity::defaultNameFromCounters()` to understand how the array  |
|                                |               | is used.                                                                   |
+--------------------------------+---------------+----------------------------------------------------------------------------+

..  Blank row:
..  |                                |               |                                                                            |

.. _bridge-model-properties:

Model properties of bridges
---------------------------

In general, bridges should choose a prefix for their property names
so that developers can easily identify the source of the property,
even when saved in a JSON model file.
The exception to this rule is properties that should be universal
across bridges, such as pedigree ID.

Properties specific to the Exodus bridge are listed in the table below.

+--------------------------------+---------------+----------------------------------------------------------------------------+
| Property name                  | Property type | Description                                                                |
+================================+===============+============================================================================+
| exodus id                      | Integer       | The block ID or set ID as stored in the Exodus file.                       |
+--------------------------------+---------------+----------------------------------------------------------------------------+
| exodus type                    | String        | One of "block", "side set", or "node set".                                 |
|                                |               | This indicates how the group is represented in the exodus file.            |
|                                |               | The group's dimension bits can also be used to determine this information  |
|                                |               | by comparing them to the parent model's parametric dimension.              |
+--------------------------------+---------------+----------------------------------------------------------------------------+
