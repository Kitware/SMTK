.. highlight:: c++
.. role:: cxx(code)
   :language: c++

.. _adding-entity-uuids:

*******************
Adding Entity UUIDs
*******************

The first step in adapting foreign modeling kernels to SMTK,
which you should do before you start subclassing Session,
is deciding how to assign UUIDs to entities in the foreign
modeling kernel so that

* entities can persist across SMTK modeling sessions and
* SMTK can identify entities to the modeling kernel given
  only a UUID (for instance, to specify inputs to a modeling
  kernel operation).

There are two ways to go about storing and maintaining this
bidirectional map between modeling kernel entities and SMTK
UUIDs:

1. If the modeling kernel provides an attribute system,
   then the UUIDs can be stored as attributes on entities.
   Note that it is important to verify that attributes
   can be stored on all the entities you wish SMTK to be
   able to track.
   For instance, CGM does not provide a way to store
   attributes on "use" records or on loops and shells.
   This means we must use another method if we want
   to preserve UUIDs assigned to these entities.

2. If the modeling kernel provides a "stable" order for
   traversing model entities, the mapping can be reduced
   to storing one or more sequences of UUIDs in an
   SMTK model file. If you construct this file so that
   it also saves session information, it can be used to
   "restore" a modeling session so that the same files
   are loaded on the server and the UUIDs preserved.

The Exodus session follows the first approach and
expects UUIDs to be saved as field-data
on each data object to be represented in SMTK.
To accelerate lookups of UUIDs,
the Exodus session stores the UUID as a string value
on each :cxx:`vtkDataObject`'s information object using the :cxx:`SMTK_UUID_KEY` key.

.. literalinclude:: ../../../smtk/bridge/exodus/ReadOperator.cxx
   :start-after: // ++ 1 ++
   :end-before: // -- 1 --

The advantage to the first approach is that modeling
kernels with attribute systems generally provide a way
to preserve attributes across modeling operations.
When using the latter method, there is no robust way to
track entities across modeling operations that change
the topology of the model.

Adding UUIDs by either technique
--------------------------------

Regardless of the path you take above, your session should provide
public methods to map both directions.
The function mapping UUIDs to foreign entities will have a
return type that is specific to your modeling kernel,
as will the input parameter of the inverse method that
returns a UUID given a foreign entity;
for our example, we've created a new type named :cxx:`EntityHandle`.

.. literalinclude:: ../../../smtk/bridge/exodus/Session.h
   :start-after: // ++ 2 ++
   :end-before: // -- 2 --

Each :cxx:`EntityHandle` instance stores information about a model
or group that should be presented in SMTK:
(1) :cxx:`m_session`,
    a pointer to the session owning the model or group;
(2) :cxx:`m_modelNumber`,
    an integer offset into the session's list of top-level models
    (this indicates the model that owns the data object); and
(3) :cxx:`m_object`,
    a pointer to a VTK dataset holding the corresponding model geometry.
The session also holds a map to identify the parent model or group of
each VTK data object since multiblock datasets in VTK do not provide
a way to discover the parent of a dataset (only its children).

Adding UUIDs as attributes
--------------------------

Although we do not provide example code in this tutorial,
it should be straightforward to add UUIDs to a modeling kernel's
attribute system either as a 16-byte binary blob or an ASCII string
per entity.

For example, if we wished to make VTK points and cells available
via a session, we could store UUIDs on VTK grids as point and cell data arrays.
It would be more space-efficient to store these in a 2-component
:cxx:`vtkTypeUInt64Array` (2 components for a total of 128 bits per UUID),
but much easier to debug if we store UUIDs in :cxx:`vtkStringArray`
instances (one for points, one for cells).

VTK provides the concept of pedigree IDs for mapping points and cells
from inputs to corresponding outputs, so (for filters that support
pedigree IDs) VTK behaves much like an attribute system in a geometric
modeling kernel.

If we ever expose individual points and cells in the SMTK Exodus session,
we could search for point- and cell-data arrays containing UUIDs.
Currently, we only search for field data specifying the UUID of a dataset
as shown above.

Although not required by this example, you should be aware that you
may store information about a particular session instance in
an SMTK JSON file by subclassing the :smtk:`SessionIOJSON` class.

.. _session-by-sequence:

Adding UUIDs as sequences
-------------------------

If you must store UUIDs in an SMTK JSON file according to some stable
traversal order, then you should

1. store the arrays in your session class in the proper order and use
   them to perform the lookups.

2. subclass :smtk:`SessionIOJSON <smtk::model::SessionIOJSON>` in order to preserve the UUID arrays.
   The Exodus session illustrates how to do this but does not currently
   export any session information.

   The Exodus session class provides a method to create an IO delegate:

   .. literalinclude:: ../../../smtk/bridge/exodus/Session.cxx
      :start-after: // ++ 12 ++
      :end-before: // -- 12 --

   The IO delegate class is then implemented to provide methods for
   importing and exporting session-specific information:

   .. literalinclude:: ../../../smtk/bridge/exodus/SessionExodusIOJSON.h
      :start-after: // ++ 1 ++
      :end-before: // -- 1 --

Note that you can use the :smtk:`smtk::bridge::exodus::SessionIOJSON` class
to import and export information other than UUID arrays, should the session
store some other state that should be preserved across processes.

Summary
-------
You should now have a way to map a foreign entity to its SMTK UUID and vice-versa.
If there is any possibility that the model could change, then you are responsible
for maintaining this mapping (or at least marking it dirty and rebuilding as
required).

Note that with either approach it is possible to incrementally assign UUIDs
to entities as the need arises (though not very efficiently when storing UUIDs
by traversal order).
Future versions of SMTK will focus on incremental transcription of
model entities to avoid significant waits the first time a model is read.

Now that you understand how UUIDs will be stored and related to foreign
modeling kernel entities, it is possible to subclass the SMTK model session class.
