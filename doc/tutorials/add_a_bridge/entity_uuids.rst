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
   *This is the approach our Exodus session example takes.*

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

The :cxx:`EntityHandle` type stores 3 integer values for each model
or group it exposes to SMTK:
(1) the type of object being exposed (an Exodus model, an element
block, a side set, or a node set),
(2) the offset of the model in a vector of vtkMultiBlockDataSet
instances held by the session (one per Exodus file)
(3) the ID of the block holding the vtkUnstructuredGrid that contains
the tessellation information for the object (or -1 when the object
is an Exodus MODEL since it has no tessellation, only groups).
Given an :cxx:`EntityHandle` we can easily look up the vtkUnstructuredGrid
in the Session's :cxx:`m_models` member.

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

Thus, when given any VTK dataset, we would first search for point and cell
arrays that define UUIDs and create them if they are not present.

Besides points and cells, we might wish to assign UUIDs to datasets
themselves and even filter objects.
This might be achieved by creating a new vtkInformationKey to hold
UUIDs and assigning it to the vtkInformation object on the dataset
or filter.

Although not required by this technique, you should be aware that you
may store information about a particular session session instance in
an SMTK JSON file by subclassing the :smtk:`SessionIOJSON` class.

.. _session-by-sequence:

Adding UUIDs as sequences
-------------------------

If you must store UUIDs in an SMTK JSON file according to some stable
traversal order, then you should

1. store the arrays in your session class in the proper order and use
   them to perform the lookups.

   .. literalinclude:: ../../../smtk/bridge/exodus/Session.cxx
      :start-after: // ++ 2 ++
      :end-before: // -- 2 --

2. subclass :smtk:`SessionIOJSON` in order to preserve the UUID arrays:

   .. literalinclude:: ../../../smtk/bridge/exodus/SessionExodusIOJSON.h
      :start-after: // ++ 1 ++
      :end-before: // -- 1 --

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
