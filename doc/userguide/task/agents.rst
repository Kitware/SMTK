.. _smtk-task-agents:

Guide to task agents
====================

Agents are responsible for monitoring the state of a project's resources
in order to respond to upstream workflow changes and to determine when
users may mark the agent's parent task completed.

The following sections describe the different agents that a task may own.

.. _task-agent:

Agent
-----

The base :smtk:`agent <smtk::task::Agent>` class is abstract and may not
be instantiated, but every concrete subclass of agent must have the
``type`` JSON configuration parameter:

* ``type``: the type-name of a concrete agent subclass to instantiate (chosen
  from those listed below.

.. _task-fill-out-attributes-agent:

FillOutAttributesAgent
----------------------

The :smtk:`FillOutAttributesAgent<smtk::task::FillOutAttributesAgent>`
monitors attribute resources provided to it either by its static configuration
or an input port (if the agent is configured with one).
When an operation creates or modifies attributes matching the agent's
configuration, the task checks whether the attributes are valid.
If so, the agent is Completable. If not, it is Incomplete.
If no attribute resources with matching attributes are configured, then
the agent is Irrelevant.

This agent accepts all the JSON configuration that the base agent class does, plus:

* ``attribute-sets``: a JSON array of required attributes, organized by role.
  Each array entry must be a JSON object holding:

    * ``role``: an optional string holding an attribute-resource role name.
      If omitted, any role is allowed.
    * ``definitions``: a set of :smtk:`smtk::attribute::Definition` type-names
      specifying which types of attributes to validate before allowing completion.
    * ``output-data``: an enumerant from :smtk:`smtk::task::FillOutAttributesAgent::PortDataObjects`
      specifying whether to broadcast resources, attributes, or both on the
      agent's output port (if any is configured). This defaults to broadcast
      only resources if omitted.
    * ``auto-configure``: either true or false (the default), depending on
      whether resources with matching roles should automatically be added.
      The default is true.
      If false, your static configuration must provide the UUID of an attribute
      or attribute resource.

Example
"""""""

.. code:: json

   {
     "type": "smtk::task::FillOutAttributesAgent",
     "attribute-sets": [
       {
         "role": "simulation attribute",
         "definitions": ["SolidMaterial", "FluidMaterial"],
         "output-data": "smtk::task::FillOutAttributesAgent::Attributes"
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

Note the ``output-data`` specifies that any valid ``SolidMaterial`` or
``FluidMaterial`` attributes should be included in the task's output
port-data (in the "simulation attribute" role) if an output port is
configured.

.. _task-submit-operation-agent:

SubmitOperationAgent
--------------------

The :smtk:`SubmitOperationAgent<smtk::task::SubmitOperationAgent>`
monitors the operation manager to ensure an operation (specified
by type-name) has run successfully since its parameters were last modified.
The agent may require an operation be run only once or iteratively
until the user finds the results acceptable.

The operation being managed by the agent is created by the agent
and its parameters are maintained as part of the agent's state.
Its parameters may also be overridden from data on input ports
provided to the agent.

Objects from the most recent successful execution of the agent's
operation are made available on the output port.

Next we'll discuss the JSON configuration parameters.
Because the number of parameters is large, we'll break this into
three groups of parameters: the minimal set required to function,
typical additional parameters workflow developers will specify, and
finally the remaining parameters, which are mostly programmatic ones
used to preserve the state of the task across modeling sessions.

Minimal Parameters
""""""""""""""""""

In addition to the ``type`` parameter inherited from the base Agent class,
you must specify these parameters for the SubmitOperationAgent to be functional:

* ``operation``: The typename of the operation the agent should create and maintain.
  If no operation is specified, this agent's state will always be irrelevant.
* ``run-style``: Determine how running the agent's operation affects the task state.
  This must be one of the following values:

  * ``smtk::task::SubmitOperationAgent::RunStyle::Iteratively`` or ``iteratively-by-user``:
    indicates the operation may be run as many times as users see fit, but it must be run
    at least once and the task will not be completable as long as the parameters have been
    edited more recently than the operation last ran.
  * ``smtk::task::SubmitOperationAgent::RunStyle::Once`` or ``once-only``:
    indicates the operation must only be run once.
    Once the operation completes successfully (by clicking ``Apply`` in the operation
    parameter-editor panel), the task is marked as completed.
    If the user un-checks the "completed" button on the task,
    the operation must be run again.
  * ``smtk::task::SubmitOperationAgent::RunStyle::OnCompletion`` or ``upon-completion``:
    indicates the operation should be run only once – immediately after the user marks
    the task as completed.
    If the operation fails, the task will return to an Incomplete state.
    The ``Apply`` button in the operation parameter-editor panel will be hidden and
    so that users must mark the task completed in order to run the operation.
    Furthermore, the task will be incomplete until the operation's parameters are in
    a valid state.

Minimal Example
~~~~~~~~~~~~~~~

.. code:: json

   {
     "type": "smtk::task::SubmitOperationAgent",
     "run-style": "smtk::task::SubmitOperationAgent::RunStyle::Once",
     "operation": "math_op.MathOp"
   }


Typical Parameters
""""""""""""""""""

In addition to the parameters above, these parameters exist to
describe how port-data should affect the parameters of the operation
managed by SubmitOperationAgent.

* ``parameters``: a map of maps holding an array of
  :smtk:`ParameterSpec<smtk::task::SubmitOperationAgent::ParameterSpec>` objects.
  The keys of the outer map are port names from the parent task.
  The keys of the inner map are role names on that port from which operation-parameter
  information should be extracted.
  The ``ParameterSpec`` values include the following parameters that describe how
  to convert port data into values for the parameter

  * ``handler``: One of the following values:

    * ``smtk::task::SubmitOperationAgent::PortDataHandler::AddObjects`` (or ``add``):
      Append the objects to the item (or associations) at the given item path.
    * ``smtk::task::SubmitOperationAgent::PortDataHandler::SetObjects`` (or ``set``):
      Reset the reference-item (or associations) at the given item path to
      the objects on the named port in the named role.
    * ``smtk::task::SubmitOperationAgent::PortDataHandler::AssignFromAttribute`` (or
      ``assign-from-attribute``):
      Assign an attribute's item to an operation-parameter's item.
    * ``smtk::task::SubmitOperationAgent::PortDataHandler::AssignFromAttributeResource`` (or
      ``assign-from-attribute-resource``):
      Find an attribute in an attribute resource and assign one of its its items to an
      item of the operation-parameter attribute.
    * ``smtk::task::SubmitOperationAgent::PortDataHandler::AssignMatchingAttributes`` (or
      ``assign-from-matching-attributes``):
      Any matching attribute resource with a matching attribute **or** any matching
      attribute should have the given item assigned to an item of the operation-parameter
      attribute. This mimics the behavior of both ``AssignFromAttribute`` and
      ``AssignFromAttributeResource``.

  * ``resource-type``: For a port-data object to be used as a parameter value, the
    object (if it is a resource) or its parent resource (if it is a component) must
    match the type-name provided here.
  * ``resource-template``: If provided, the port-data object's resource must have
    a matching template-type (see :smtk:`Resource::templateType()<smtk::resource::Resource::templateType>`)
    in order to provide values to the operation.
  * ``component-selector``: A filter-query string (passed to
    :smtk:`Resource::queryOperation()<smtk::resource::Resource::queryOperation>`) that
    specifies objects to extract from a resource on the port-data.
    This is used to select attributes from an attribute resource whose values should be
    provided to an operation parameter.
  * ``source-path``: The path to an attribute-item whose value should be assigned to
    an operation parameter. This is ignored when ``handler`` is ``AddObjects`` or ``SetObjects``.
  * ``target-path``: The path to an attribute-item in the operation's parameters.
    This is the item to which values should be assigned. If ``handler`` is ``AddObjects``
    or ``SetObjects``, the item at this path must be a reference-item (or the name of
    the associations for the operation).
  * ``configured-by``: One of the following

    * ``smtk::task::SubmitOperationAgent::ConfiguredBy::Static`` (or ``static``):
      The operation parameter should be fixed to the value in ``op-params``.
    * ``smtk::task::SubmitOperationAgent::ConfiguredBy::Port`` (or ``port``):
      The operation parameter should be pinned to the port data specified
      via the parameters above.
    * ``smtk::task::SubmitOperationAgent::ConfiguredBy::User`` (or ``user``):
      The user may override values provided by the input port.
      Once the user overrides the port-data, the ``ParameterSpec`` is marked
      to prevent port-data from being assigned.
      **WARNING: This is not implemented yet.**

  * ``user-override``: ``true`` when the user-provided value overrides the port
    data and ``false`` otherwise.
  * ``visibility``: Choose whether the operation parameter at ``target-path`` is
    visible to the user in the operation parameter-editor panel. One of the
    following values must be chosen if this is specified.
    **WARNING: This is not implemented yet.**

    * ``smtk::task::SubmitOperationAgent::ItemVisibility::On``:
      The item (and its children, if any) should be visible in the parameter editor.
    * ``smtk::task::SubmitOperationAgent::ItemVisibility::Off``:
      The item (but not any children marked as visible, if any) should be hidden in the parameter editor.
    * ``smtk::task::SubmitOperationAgent::ItemVisibility::RecursiveOff``:
      The item (and all children) should be hidden in the parameter editor.

* ``output-port``: is the name of an output port on which this agent should provide
  a set of resources mentioned in the operation's result object.
  Resources are only available to the output port when the operation succeeds;
  if the operation fails, the list of resources will be cleared.
  In either case, the port will be notified its data has changed.
  The port name is optional, but if provided you must also provide ``output-role``.
* ``output-role``: is the role in which operation results should be added to
  output port-data.

Typical Example
~~~~~~~~~~~~~~~

.. code:: json

   {
     "type": "smtk::task::SubmitOperationAgent",
     "run-style": "smtk::task::SubmitOperationAgent::RunStyle::Once",
     "operation": "math_op.MathOp",
     "parameters": [
       [
         "parameters",
         [
           [
             "attributes",
             [
               {
                 "component-selector": "attribute[type='Stage1Data']",
                 "configured-by": "smtk::task::SubmitOperationAgent::ConfiguredBy::User",
                 "handler": "smtk::task::SubmitOperationAgent::PortDataHandler::AssignMatchingAttributes",
                 "resource-type": "smtk::attribute::Resource",
                 "source-path": "/DoubleValue",
                 "target-path": "/DoubleValue"
               },
               {
                 "component-selector": "attribute[type='Stage1Data']",
                 "configured-by": "smtk::task::SubmitOperationAgent::ConfiguredBy::Port",
                 "handler": "smtk::task::SubmitOperationAgent::PortDataHandler::AssignMatchingAttributes",
                 "resource-type": "smtk::attribute::Resource",
                 "source-path": "/IntValue",
                 "target-path": "/IntValue",
                 "visibility": "smtk::task::SubmitOperationAgent::ItemVisibility::On"
               }
             ]
           ]
         ]
       ]
     ],
     "output-port": "results",
     "output-role": "stage 1 math op"
   }

Remaining Parameters
""""""""""""""""""""

These parameters are mostly values written when users save a project's state
but are not necessarily of interest to workflow developers.
However, there is nothing precluding workflow developers from using these:

* ``run-since-edited``: ``true`` when the operation has been run since the parameters
  have been modified and ``false`` otherwise. This defaults to false.
* ``internal-state``: records the state at the time the project was saved. This
  allows projects to "remember" whether the operation has been successfully run.
* ``op-params``: A serialized :smtk:`attribute<smtk::attribute::Attribute>` holding the
  operation's parameters. If none is provided, the default parameters are used.
* ``op-links``: A serialized set of :smtk:`resource links<smtk::resource::ResourceLinks>`
  used to hold association and reference-item values for the operation's parameters.
  If none is provided, associations and reference-items will be unset.
* ``watching``: is a map holding the UUIDs of persistent objects that, when modified,
  should be used to update the operation's parameters. This is used to keep the task's
  operation-parameters up-to-date when objects provided via upstream port-data are
  modified by other operations.
* ``output-resources``: is an array of UUIDs of resources provided from any
  prior, successful run of the operation. This allows the output port-data (if
  one has been configured) to be preserved across sessions so that even when the
  parent task is completed in a previous session, the port-data is available in
  the current and future sessions.

Complete Example
~~~~~~~~~~~~~~~~

.. code:: json

   {
     "type": "smtk::task::SubmitOperationAgent",
     "run-style": "smtk::task::SubmitOperationAgent::RunStyle::Once",
     "run-since-edited": true,
     "operation": "math_op.MathOp",
     "internal-state": "completable",
     "op-links": null,
     "op-params": {
       "ID": "96873123-1758-439d-990d-09636ff55b79",
       "Items": [
         {
           "Enabled": false,
           "ForceRequired": false,
           "Name": "debug level",
           "SpecifiedVal": "0",
           "Val": 0
         },
         {
           "Name": "IntValue",
           "SpecifiedVal": "8",
           "Val": 8
         },
         {
           "Name": "DoubleValue",
           "SpecifiedVal": "49",
           "Val": 49.0
         }
       ],
       "Name": "math_op.MathOp8",
       "Type": "MathOp"
     },
     "parameters": [
       [
         "parameters",
         [
           [
             "attributes",
             [
               {
                 "component-selector": "attribute[type='Stage1Data']",
                 "configured-by": "smtk::task::SubmitOperationAgent::ConfiguredBy::User",
                 "handler": "smtk::task::SubmitOperationAgent::PortDataHandler::AssignMatchingAttributes",
                 "resource-type": "smtk::attribute::Resource",
                 "source-path": "/DoubleValue",
                 "target-path": "/DoubleValue"
               },
               {
                 "component-selector": "attribute[type='Stage1Data']",
                 "configured-by": "smtk::task::SubmitOperationAgent::ConfiguredBy::Port",
                 "handler": "smtk::task::SubmitOperationAgent::PortDataHandler::AssignMatchingAttributes",
                 "resource-type": "smtk::attribute::Resource",
                 "source-path": "/IntValue",
                 "target-path": "/IntValue",
                 "visibility": "smtk::task::SubmitOperationAgent::ItemVisibility::On"
               }
             ]
           ]
         ]
       ]
     ],
     "watching": [
       [
         "5cd08eef-ee59-477d-8eec-c7cacd768deb",
         [
           {
             "idx": 0,
             "port": "parameters",
             "role": "attributes"
           },
           {
             "idx": 1,
             "port": "parameters",
             "role": "attributes"
           }
         ]
       ]
     ]
   }

The JSON above is based on the example project in ``data/projects/SimpleWorkletExample/agentWorklet.smtk``
so that you can load the project and explore how the mentioned UUIDs relate to the attribute resource
included in the project.

GatherObjectsAgent
------------------

The :smtk:`GatherObjectsAgent<smtk::task::GatherObjectsAgent>`
simply collects persistent objects with roles and makes them available
on the specified output port of the parent task.

To add objects to the agent programmatically, you should
call :smtk:`GatherObjectsAgent::addObjectInRole()<smtk::task::GatherObjectsAgent::addObjectInRole>`,
:smtk:`GatherObjectsAgent::removeObjectFromRole()<smtk::task::GatherObjectsAgent::removeObjectFromRole>`,
or :smtk:`GatherObjectsAgent::setObjectsInRoles()<smtk::task::GatherObjectsAgent::setObjectsInRoles>`.
Note that you should *not* provide the broadcast port unless this method is being called from an observer.
If you call these methods from within an operation, simply add the modified port to the operation's
results.

The following JSON configuration parameters are available for
this agent:

* ``output-port``: is the name of the parent-task's port on which this agent
  should broadcast its data.
* ``objects``: is a map from a role name to a set of persistent-object specifiers.
  If an object is a resource, each specifier is a tuple holding a UUID and ``null``.
  If an object is a component, each specified is a tuple holding the UUID of the
  component's parent resource and the component's UUID.

Example
"""""""

.. code:: json

   {
     "type": "smtk::task::GatherObjectsAgent",
     "output-port": "gathered resources",
     "objects": [
       [ "thermal simulation parameters",
         [
           [ "96873123-1758-439d-990d-09636ff55b79", null ]
         ],
       ],
       [ "thermal materials",
         [
           [ "96873123-1758-439d-990d-09636ff55b79", "e87115f1-585b-4572-b81b-289b4db5e5cd" ],
           [ "96873123-1758-439d-990d-09636ff55b79", "61418aa2-f130-4472-8ca0-e9e7b3c622d3" ]
         ]
       ]
     ]
   }

The JSON above provides a resource in the "thermal simulation parameters" role
and two components from within that resource in the "thermal materials" role.
