.. _smtk-model-sys:

*****************************
SMTK's Geometric Model System
*****************************

SMTK's second major component is its geometric modeling system,
which provides bridges to multiple solid modeling kernels.

Key Concepts
============

Like the attribute system, the model system is composed of C++ classes,
also accessible in Python, whose instances perform the following functions:

:smtk:`Manager <smtk::model::Manager>`
  instances that contain model topology and geometry.
  All of the model entities such as faces, edges, and vertices are
  assigned a UUID by SMTK.
  You can think of the manager as a key-value store from UUID values to
  model entities, their properties, their arrangement with other entities,
  their ties to the attribute system, and their tessellations.

:smtk:`Bridge <smtk::model::Bridge>`
  instances relate entries in a model Manager to a solid modeling kernel.
  You can think of the entities in a model Manager as being "backed" by
  a solid modeling kernel; the bridge provides a way to synchronize
  the representations in the Manager and the modeling kernel.
  A manager may contain entity records from multiple Bridge sessions
  (e.g., a single Manager may contain some models back by an ACIS
  modeling kernel bridge and some backed by OpenCascade bridge).

:smtk:`Operator <smtk::model::Operator>`
  instances represent modeling operations that a modeling kernel
  provides for marking up, modifying, or even generating modeling entities
  from scratch.
  Operators usually require the entries in the model Manager to be
  updated after they are executed (in the solid modeling kernel).
  Each operator implements a method to invoke its operation in the modeling kernel
  and owns an attribute system Attribute instance (its *specification*) to store
  the parameters required by the operation.
  SMTK expects the primary operand of an operator (e.g., a set of edge entities
  in the model manager to be swept into a face) to be model entities
  **associated** with the operator's specification.
  Other operands (e.g., the geometric path along which to sweep a set of edges)
  are stored as key-value Items in the specification.

:smtk:`Cursor <smtk::model::Cursor>`
  instances are lightweight references into a model Manager's storage
  that represent a single entity (e.g., a vertex, edge, face, or volume)
  and provide methods for easily accessing related entities, properties,
  tessellation data, and attributes associated with that entity.
  They also provide methods for manipulating the model Manager's storage
  but *these methods should not be used directly*; instead use an Operator
  instance to modify the model so that the kernel and manager stay in sync.
  Cursor subclasses include Vertex, Edge, Face, Volume, ModelEntity,
  GroupEntity, UseEntity, Loop, Shell, and so on. These are discussed
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

As mentioned above, the model :smtk:`Manager <smtk::model::Manager>` class is the only place where
model topology and geometry are stored in SMTK.
However, there are cursor-like classes, all derived from :smtk:`smtk::model::Cursor`,
that provide easier access to model traversal.
These classes are organized like so:

.. findfigure:: cursor-classes-with-inheritance.*

   Each of the orange, green, purple, and red words is the name of a cursor class.
   The black arrows show relationships between instances of them (for which the
   classes at both ends provide accessors).

Each relationship shown in the figure above has a corresponding
method in the cursor subclasses for accessing the related entities.

Bridges
=======

As mentioned above, :smtk:`Bridges <Bridge>` link, or *back* SMTK model entities
to a solid-modeling kernel's representation of those model entities.
Not all of the model entities in a model manager need to be backed by the same bridge;
SMTK can track models from ACIS and OpenCascade in the same model manager.
However, in general you cannot perform modeling operations using entities from different bridges.

Bridges (1) transcribe modeling-kernel entities into SMTK’s storage and
(2) keep a list of :smtk:`Operators <Operator>` that can be used to modify the model.
As part of the transcription process, bridges track which entities have been incompletely transcribed,
allowing partial, on-demand transcription.
SMTK’s existing bridges (to CGM and CMB’s discrete modeler) use the attribute systems
those modelers provide to hold SMTK-generated universal, unique IDs (UUIDs) for each model entity;
modeling-kernel bridges may also provide a list of UUIDs in an unambiguous traversal order
if UUIDs cannot be stored in a model file.

When a model operation is performed,
— depending on how much information the modeling kernel provides about affected model entities —
entities in SMTK’s storage are partially or totally marked as dirty and retranscribed on demand.


Registration and initialization of Bridges and Operators
--------------------------------------------------------

Because bridges usually back SMTK model entities with representations in a solid
modeling kernel, constructing a bridge (and thus initializing a modeling kernel)
can be an expensive operation.
This expense is even higher if the actual bridge must live in a separate process
or even possibly on a remote machine.
Because applications need to be able to discover what bridges are available and
provide enough information to users to make an informed decision about which one
to use, metadata on available bridges is stored in the model manager to avoid
the overhead of constructing an instance of each available bridge type.

There are two ways for bridge metadata to get registered with the model manager:

* via static initialization at link time
  (using the :smtk:`smtkImplementsModelingKernel` macro),
* via dynamic registration at run time
  (using the :smtk:`BridgeRegistrar::registerBridge` method).

This is further complicated by the fact that Operator subclasses need to
be registered with particular Bridge subclasses — and this registration may also
occur at link time (so that developers can concisely specify the Operator-Bridge
association as part of the Operator's implementation) or at run time (so that
composite operators such as macros can be created on the fly; and so that
modeling kernels that provide programmatic access to their operators can have
their operators enumerated in SMTK at run time).

Finally, because bridges and their operators are maintained in separate libraries
and static initialization is very platform-dependent, some extra work may be
required in order to ensure that statically-registered bridges have had a chance
to declare themselves before applications query SMTK for a list of bridges.
By placing the :smtk:`smtkComponentInitMacro` in your application's
main source file:

.. highlight:: c++
.. literalinclude:: ../../smtk/bridge/remote/smtk-remote-model.cxx
  :start-after: // ++ UserGuide/Model/1 ++
  :end-before: // -- UserGuide/Model/1 --
  :linenos:

you can ensure that the named component (the CGM bridge in this example) has
its initializer called at the same time that static variables for your application
are initialized — even on platforms that perform lazy dynamic linking or provide
SMTK as static libraries.
Note that you do **not** need to include any header files declaring the components
you wish initialized; the smtkComponentInitMacro declares a global C function that
each component should provide.

Remote models
=============

For many reasons (e.g., incompatible library dependencies, licensing issues, distributed processing),
it is often necessary for the modeling kernel to live in a different process than other portions of
the simuation pipline.

SMTK allows this by implementing special bridge and operator classes
that serialize operators and send them to a remote process
where the usual bridge for that type of model entity exists.
The usual bridge then performs the operation and sends the results
back to the originating process, as diagrammed below.

All bridge classes that will forward operators to other bridges must
inherit from the :smtk:`DefaultBridge` class instead of :smtk:`Bridge`.
The DefaultBridge class always creates :smtk:`RemoteOperator` instances
when asked for an operator by name;
the RemoteOperator class delegates its ableToOperate and operate methods
to the DefaultBridge instance which instantiated it.

.. findfigure:: forwarding-bridge.*

   The CMB client-server model uses SMTK's RemoteOperator and DefaultBridge classes to
   forward operations from the client to the server (and results back to the client).

If you want to use this functionality in your application,
the action diagram below illustrates the sequence of events that
take place.

.. actdiag::
  :caption: An action diagram of how operators are forwarded from your
            application to a model worker for the appropriate modeling kernel.

  actdiag {

    request_op -> instantiate_op -> prepare_op ->
    apply_op1 -> serialize_op -> deserialize_op ->
    apply_op2 -> serialize_result -> deserialize_result ->
    update_manager -> present_result

    lane app {
      label = "Your Application"
      request_op [label="Request operator\n by name"]
      prepare_op [label="Prepare operator\n attribute values"]
      apply_op1 [label="Apply operator"]
      present_result [label="Present result\nof operation"]
    }
    lane bridge {
      label = "Forwarding Bridge"
      instantiate_op [label = "Instantiate an\n operator attribute"]
      serialize_op [label = "Serialize operator\n attribute"]
      deserialize_result [label = "Serialize operator\n result"]
      update_manager [label = "Update model manager\n as required"]
    }
    lane server {
      label = "Server Bridge"
      deserialize_op [label = "Deserialize operator\n attribute"]
      apply_op2 [label = "Run operation in\n modeling kernel"]
      serialize_result [label = "Serialize operator\n result"]
    }
  }

Remote operators behave identically to their actual counterparts,
so your application does not need special logic to deal with entities
from remote bridges.
However, your application must help SMTK discover remote processes
that are available for solid modeling.

.. todo::

  Discuss Remus and ParaView client/server bridges
