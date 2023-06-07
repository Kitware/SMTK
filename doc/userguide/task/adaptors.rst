.. _smtk-adaptor-classes:

Guide to adaptor classes
========================

The following sections describe the different adaptor subclasses that
exist for use in your workflows, the use cases that motivate them,
and how to configure them.

Adaptor
-------

The :smtk:`base Adaptor class<smtk::task::Adaptor>` is abstract and
cannot be instantiated on its own but does have configuration parameters
that are common to all adaptors and must be provided:

* ``id``: this is an integer identifying the adaptor.
  Although currently unused, all adaptor JSON should provide a unique,
  non-zero value as it may be used in the future to reference adaptors
  during configuration.
* ``type``: this is the fully-qualified class name of the adaptor
  class that should be created.
* ``from``: an integer used to identify the task which should be
  observed and used to configure another.
* ``to``: an integer used to identify the task which should be
  configured when the "from" task has changed into a completable
  or completed state.

Example
"""""""

.. code:: json

   {
     "id": 1,
     "type": "smtk::task::Adaptor",
     "from": 1,
     "to": 2
   }

ResourceAndRole
---------------

The :smtk:`ResourceAndRole <smtk::task::adaptor::ResourceAndRole>` adaptor
exists to pass information from GatherResources to FillOutAttributes,
possibly passing this information "through" Groups that have these tasks
as children.
When configured, only attribute resources chosen by the user will be examined
for validity.

The ResourceAndRole adaptor accepts two additional JSON configuration entries
beyond the base Adaptor class:

* ``from-tag``: this is a string used *only* when the "from" task is a Group.
  It specifies a key in the Group's interal configuration object from which
  to fetch data to apply to the destination task.
  The default value is an empty string.
* ``to-tag``: this is a string used *only* when the "to" task is a Group.
  It specifies a key in the Group's interal configuration object to which
  to write data that will be applied to the destination task.
  The default value is an empty string.

The Group from- and to-tag strings exist because a Group task may need
to aggregate state from multiple child tasks or configure multiple
child tasks separately.
By providing the key, authors can control which child tasks share or
do not share configuration information.
If the default empty string is used, then all tasks will share
the same configuration information.

Example
"""""""

.. code:: json

   {
     "id": 1,
     "type": "smtk::task::adaptor::ResourceAndRole",
     "from": 1, /* must be a GatherResources or Group task */
     "to": 2,   /* must be a FillOutAttributes or Group task */
     "to-tag": "foo" /* if "to" is a Group, "foo" is a key for resource+role data. */
   }


ConfigureOperation
------------------

The :smtk:`ConfigureOperation <smtk::task::adaptor::ConfigureOperation>`
adaptor is used to automatically configure parts of an SMTK operation
assigned to a ``smtk::task::SubmitOperation`` task.
When the ``from`` task transitions to the ``Completable`` state,
this adaptor will copy values from attribute sets referenced in the
source task ( ``smtk::task::FillOutAttributes`` ) to operation parameters
in the destination task. And if the source task is active and in the
``Completable`` state, changes made to those attribute sets using the
smtk view system will also be forwarded.

The JSON object to configure a ``ConfigureOperation`` adaptor adds a
``configure`` element to those defined for the base Adaptor class.
The ``configure`` element is a JSON array with each child element a
JSON object.

* Each element in the ``configure`` array must include a ``from-role``
  element to
  specify the attribute resource (matching the ``role`` in the
  ``smtk::task::FillOutAttributes`` task).
* Additional key-value pairs specify the items to copy from
  the attribute resource to the operation parameter instance.

  * Each key specifies one item in the attribute resource, using the
    SMTK grammar syntax for search criteria.
    These keys always start with the ``attribute[]`` syntax.
  * Each value specifies the corresponding item in the operation
    parameters. Because the parameter attribute is known to the
    operation, the ``attribute[]`` syntax is omitted from this
    part of the specification.


Example
""""""""

.. code:: json

    {
      "id": 1,
      "type": "smtk::task::adaptor::ConfigureOperation",
      "from": 1,  /* must be a FillOutAttributes task */
      "to": 2,    /* must be a SubmitOperation or Group task */
      "configure":
        [
          {
            "from-role": "simulation attributes",
            "attribute[type='BoxWidget']/box": "/bounds"
            "attribute[type='BlockMeshSize']/MeshSize": "/meshsize",
            "attribute[type='BlockMeshBoundaryConditions']/FrontBackSides": "/bc/frontback"
          },
          {
            ...
          }
        ]
    }
