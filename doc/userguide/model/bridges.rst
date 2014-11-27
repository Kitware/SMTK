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
.. literalinclude:: ../../../smtk/bridge/remote/smtk-model-worker.cxx
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
