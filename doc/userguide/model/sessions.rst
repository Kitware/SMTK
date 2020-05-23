Sessions
========

As mentioned above, :smtk:`Sessions <Session>` link, or *back* SMTK model entities
to a solid-modeling kernel's representation of those model entities.
Not all of the model entities in a model manager need to be backed by the same session;
SMTK can track models from multiple modeling kernels in the same model manager.
However, in general you cannot perform modeling operations using entities from different sessions.

Sessions (1) transcribe modeling-kernel entities into SMTK’s storage and
(2) keep a list of :smtk:`Operators <Operator>` that can be used to modify the model.
As part of the transcription process, sessions track which entities have been incompletely transcribed,
allowing partial, on-demand transcription.
SMTK’s opencascade and discrete session types use their respective attribute resource's
modeler to hold SMTK-generated universal, unique IDs (UUIDs) for each model entity;
modeling-kernel sessions may also provide a list of UUIDs in an unambiguous traversal order.
This is useful if UUIDs cannot be stored in a model file but also in the event where
you do not wish to modify the file by rewriting it with UUID attributes included.

When a model operation is performed,
— depending on how much information the modeling kernel provides about affected model entities —
entities in SMTK’s storage are partially or totally marked as dirty and retranscribed on demand.

Transcription involves mapping :smtk:`unique identifiers <UUID>` to
:smtk:`Entity` records, to :smtk:`Tessellation` records, to :smtk:`Arrangement`
records, and to property dictionaries.
A property dictionary maps a name to a vector of string, floating-point, and/or integer values
of arbitrary length per model entity.
While property names are arbitrary, there are some special property names used by the
modeling system:

* :smtk:`SMTK_GEOM_STYLE_PROP` marks models with a :smtk:`ModelGeometryStyle` enum
  indicating whether the model is discrete or parametric.
* :smtk:`SMTK_TESS_GEN_PROP` marks cells that have tessellations with an integer
  "generation" number indicating the age of the tessellation.
* :smtk:`SMTK_MESH_GEN_PROP` marks cells that have an analysis mesh with an integer
  "generation" number indicating the age of the mesh.

When a property value can be reliably determined by a session's modeling kernel
(independent of the model manager), the session should add that property name to the list
reported to the model manager for erasure when a model entity is being deleted.
(Other user-assigned properties are not deleted by default when an entity is erased.)

Registration and initialization of Sessions and Operators
---------------------------------------------------------

Because sessions usually back SMTK model entities with representations in a solid
modeling kernel, constructing a session (and thus initializing a modeling kernel)
can be an expensive operation.
This expense is even higher if the actual session must live in a separate process
or even possibly on a remote machine.

.. todo::

  Update how modeling sessions are exposed via plugins.


.. todo::

  Discuss Remus and ParaView client/server sessions
