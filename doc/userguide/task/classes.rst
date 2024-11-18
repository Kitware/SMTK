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
* ``style``: an optional array of strings holding presentation
  style-class names for the task.
* ``completed``: an optional boolean value indicating whether the
  user has marked the task complete or not.
* ``strict-dependencies``: an optional boolean value indicating
  whether dependencies are strict or lax (i.e., whether a task
  with dependencies will be available before its dependents are
  marked completed). The default is false.

Example
"""""""

.. code:: json

   {
     "type": "smtk::task::Task",
     "title": "Instructions to users.",
     "style": [ "unique-component-colors", "fancy-menu" ],
     "completed": false,
     "strict-dependencies": true
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

GatherResources
---------------

The :smtk:`GatherResources <smtk::task::GatherResources>` class monitors
a resource manager and is incomplete until its configured list of required
resources is acceptable, at which time it transitions to completable.
It is Incomplete by default unless unconfigured (in which case it is Completable).
It accepts all the JSON configuration that the base Task class does, plus:

* ``auto-configure``: either true or false (the default), depending on whether
  resources should be automatically pulled from the resource manager based on
  their roles (true) or whether a user must explicitly assign resources (false).
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

.. _task-submit-operation:

SubmitOperation
---------------

The :smtk:`SubmitOperation <smtk::task::SubmitOperation>` task creates an operation,
optionally pre-configures a subset of its
parameters, and may allow users to run the operation once or repeatedly.

The SubmitOperation task computes its internal state to be:

* irrelevant if no ``operation`` type-name is configured (or no operation by that
  name is registered to the application's operation manager);
* unavailable if associations or parameters are configured by a task
  adaptor (via the ``configured-by="adaptor"`` setting) and invalid. (future)
* incomplete while the operation's ``ableToOperate()`` method returns
  false; and
* completable once

  * ``run-style`` is "iteratively-by-user" or "once-only" and the operation has run successfully or
  * ``run-style`` is "upon-completion" and the operation's ``ableToOperate()`` returns true.

It accepts all the JSON configuration that the base Task class does, plus:

* ``operation``: the type-name of the operation to be created and monitored;
* ``run-style``: one of the following enumerants specifying how users should interact with the operation:

  * ``iteratively-by-user``: the operation may be run multiple times at the user's request.
  * ``once-only``: the operation may only be run once; as soon as it successfully completes,
    the operation is marked complete.
  * ``upon-completion``: the operation is not run by the user but instead is launched when the
    task is marked complete. (If the operation fails, then the task will transition back to
    completable.)
* ``run-since-edited``: false before the operation has run successfully; then, true after the operation
  has successfully run until the operation's parameters have been modified (by the task, an adaptor, or
  the user) – at which point it becomes false again.
  This is used to make the task's state consistent across a save, restart, and load of modelbuilder.

Configuration features planned for the future include the following:

* (**future**) ``associations``: a JSON object specifying how the operation's associations should be configured.
  The key-value pairs in the object may be any configuration that items in
  the ``parameters`` section above describes.
* (**future**) ``configured-by``: when set to ``adaptor``, indicates that the task associations and/or
  parameters are configured by a task adaptor. This feature is not currently enforced by smtk, but
  can be used for documentation purposes.
* (**future**) ``parameters``: an array of JSON objects that configure individual items in the operation
  parameters. Each JSON object contains some subset of the following entries:

  * ``item``: (string) item-path to the operation's parameter. This entry is *required*.
  * ``enabled``: true or false indicated whether an optional item is enabled or not.
    This is ignored if the item is not optional.
  * ``value``: a JSON array of values to store in the parameter's item.
    Specifying this forces ``enabled`` to be true.
  * ``configured-by``: one of the following enumerants specifying how the item may be edited:

    * ``task``: the item is configured solely by the provided, static values in this task's configuration.
      Adaptors will ignore items marked with this enumerant.
      By default, items marked with this enumerant are recursively hidden from the user.
    * ``adaptor``: the item is configured by one or more :smtk:`adaptors <smtk::task::Adaptor>`.
      By default, items marked with this enumerant are recursively hidden from the user.
    * ``user``: the item is expected to be edited by the user even if the task configuration or a task
      adaptor also edit the item.
      Adaptors will configure items marked with this enumerant;
      to prevent adaptors from editing an item, remove its item-path from the ``parameters`` section.
      By default, items marked with this enumerant are shown to the user.
  * ``visibility``: specifies whether the item (and optionally its children) should be shown to or
    hidden from users. Normally, this behavior is controlled by ``configured-by``, but you may
    override it explicitly by specifying one of the following enumerants:

    * ``recursive-off``: hide this item and all of its children recursively.
    * ``off``: hide only this item but show its children.
    * ``on``: show this item and its children.
  * ``role``: :smtk:`reference items <smtk::attribute::ReferenceItem>` may be provided with a role so that
    the :smtk:`ConfigureOperation <smtk::task::adaptor::ConfigureOperation>` task-adaptor
    can copy references to persistent objects into its ``value`` array.

Example
"""""""

.. code:: json

   {
     "type": "smtk::task::SubmitOperation",
     "title": "Generate a face from corner points.",
     "operation": "smtk::session::polygon::CreateEdgeFromPoints",
     "run-style": "iteratively-by-user",
     "associations": {
         "role": "model geometry",
         "configured-by": "adaptor",
         "value": []
     },
     "parameters": [
        {
          "item": "/pointGeometry",
          "value": [3],
          "configured-by": "task",
          "visibility": "off"
       }
     ]
   }

See the :ref:`smtk-pv-parameter-editor-panel` documentation for how
the user interface supports SubmitOperation tasks.
