.. _smtk-task-classes:

Guide to task classes
=====================

The following sections describe the different task subclasses that
exist for use in your workflows, the use cases that motivate them,
and how to configure them.

Task
----

The :smtk:`base Task class<smtk::task::Task>` does not monitor
anything on its own but can be used to collect dependencies.
It is Completable by default.
The following JSON can be used to configure it:

* ``title``: an optional string value holding instructions to users.
* ``completed``: an optional boolean value indicating whether the
  user has marked the task complete or not.

Example:

.. code:: json

   {
     "type": "smtk::task::Task",
     "title": "Instructions to users.",
     "completed": false
   }

TaskNeedsResources
------------------

The :smtk:`TaskNeedsResources <smtk::task::TaskNeedsResources>` class monitors
a resource manager and is incomplete until its configured list of required
resources is acceptable, at which time it transitions to completable.
It is Incomplete by default unless unconfigured (in which case it is Completable).
It accepts all the JSON configuration that the base Task class does, plus:

* ``resources``: a JSON array of required resources, organized by role.
  Each array entry must be a JSON object holding:

  * ``role``: an optional string holding a resource role name. If omitted, any role is allowed.
  * ``type``: an optional string holding a resource typename. If omitted, any resource type is allowed.
  * ``min``: an optional integer specifying the number of resources with the given role and type that must be present.
    Only non-negative values are accepted.
    It defaults to 1, which makes the requirement mandatory.
    If set to 0, the requirement is optional.
  * ``max``: an optional integer specifying the maximum number of resources with the given role and type allowed.
    Negative values indicate that there is no maximum.
    It defaults to -1.
    It is possible to set this to 0 to indicate that resources of a given role/type are disallowed.

Example:

.. code:: json

   {
     "type": "smtk::task::TaskNeedsResources",
     "title": "Load a geometric model (or models) and a simulation template.",
     "resources": [
       {
         "role": "model geometry",
         "type": "smtk::model::Resource"
       },
       {
         "role": "simulation attribute",
         "type": "smtk::attribute::Resource",
         "max": 1
       }
     ]
   }
