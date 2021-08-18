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

Example
"""""""

.. code:: json

   {
     "type": "smtk::task::Task",
     "title": "Instructions to users.",
     "completed": false
   }

Group
-----

A task :smtk:`Group <smtk::task::Group>` exists to organize a set of tasks.

The Group instance is responsible for configuring its children, including
creating dependencies among them. The Group's state and output are
dependent on its children.

The Group has a "mode," which describes how children are related to
one another: when the mode is parallel, children have no dependency on
one another; the parent group configures them independently.
When the mode is serial, children must be completed in the
order specified (i.e., each successive task is dependent on its
predecessor) and each child task may configure its successor as
it becomes completable.

Task groups are completable by default (i.e., when no children are configured).
If children exist, the group takes its internal state as a combination of its children's
states:

* irrelevant if all of its children are irrelevant;
* unavailable if all of its children are unavailable;
* incomplete if any of its children are incomplete;
* completable if all of its relevant children are completable; and
* completed when the user marks either it or all of its children completed.

As with other task classes, the group's overall state also includes the state of
its external dependencies.

The task Group class accepts all the JSON configuration that the base Task class does, plus:

* ``mode``: either ``serial`` or ``parallel``.
* ``children``: an ordered JSON array of child task specifications.
  Each child task may have an integer ``id`` whose value may be referenced
  by ``adaptors`` below.
* ``adaptors``: an array of task-adaptor specifications that inform
  the group task how to configure children. The reserved ``id`` of 0
  refers to the Group itself.

Example
"""""""

.. code:: json

   {
     "type": "smtk::task::Group",
     "title": "Perform the child tasks in order.",
     "mode": "serial",
     "children": [
       {
         "id": 1,
         "type": "smtk::task::Task",
         "title": "Step 1."
       },
       {
         "id": 2,
         "type": "smtk::task::Task",
         "title": "Step 2."
       }
     ],
     "adaptors": [
       {
         "//": "How the parent configures its child."
         "type": "smtk::task::adaptor::PassResources",
         "from": 0,
         "to": 1
       },
       {
         "//": "How the parent configures its child."
         "type": "smtk::task::adaptor::PassResources",
         "from": 0,
         "to": 2
       },
       {
         "//": "How the serial task configures its successor."
         "type": "smtk::task::adaptor::PassComponents",
         "from": 1,
         "to": 2
       },
       {
         "//": "How a child task configures its parent's"
         "//": "output. Be careful to avoid loops."
         "type": "smtk::task::adaptor::PassComponents",
         "from": 2,
         "to": 0
       }
     ]
   }


GatherResources
---------------

The :smtk:`GatherResources <smtk::task::GatherResources>` class monitors
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

Example
"""""""

.. code:: json

   {
     "type": "smtk::task::GatherResources",
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

FillOutAttributes
-----------------

The :smtk:`FillOutAttributes task <smtk::task::FillOutAttributes>`
monitors operations for attribute resources with particular roles.
When an operation creates or modifies a matching resource, the
task checks whether all the attributes with matching definitions
are valid. If so, the task is Completable. If not, it is Incomplete.
It is Completable by default (i.e., if no matching resources
or attributes exist).

This task accepts all the JSON configuration that the base Task class does, plus:

* ``attribute-sets``: a JSON array of required attributes, organized by role.
  Each array entry must be a JSON object holding:

    * ``role``: an optional string holding an attribute-resource role name.
      If omitted, any role is allowed.
    * ``definitions``: a set of :smtk:`smtk::attribute::Definition` type-names
      specifying which types of attributes to validate before allowing completion.
    * ``auto-configure``: either true or false (the default), depending on
      whether resources with matching roles should automatically be added.
      The default is false since a task-adaptor, such as
      :smtk:`ResourceAndRole <smtk::task::adaptor::ResourceAndRole>`, will
      normally configure only those resources identified by a user as
      relevant in a dependent task.

Example
"""""""

.. code:: json

   {
     "type": "smtk::task::FillOutAttributes",
     "title": "Assign materials and mesh sizing.",
     "attribute-sets": [
       {
         "role": "simulation attribute",
         "definitions": ["SolidMaterial", "FluidMaterial"]
       },
       {
         "role": "meshing attribute",
         "definitions": [
           "GlobalSizingParameters",
           "FaceSize",
           "EdgeSize"
         ]
       }
     ]
   }

In the example above, you can see that two different attribute resources
(one for the simulation and one for a mesh generator) are specified with
different roles and the definitions that should be checked for resources
in those roles are different.
