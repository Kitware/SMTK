.. highlight:: c++
.. role:: cxx(code)
   :language: c++

.. _subclassing-bridge:

**************************
Creating a bridge subclass
**************************

Bridges exist to link foreign modeling entities to SMTK
modeling entities, in a bidirectional way:

* we *transcribe* foreign modeling entities into an SMTK model manager, and
* we perform *operations* in SMTK that make changes in the foreign modeling
  kernel (and then result in more transcriptions to update SMTK's model manager).

Only the first of these is needed for read-only access so we will cover it
first and then describe the interactions between bridges and operators.
Implementing operators is the topic of another tutorial.

The first thing you must do when creating your own bridge is
to implement a subclass of :smtk:`smtk::model::Bridge`:

.. literalinclude:: Bridge.h
   :start-after: // ++ 1 ++
   :end-before: // -- 1 --

In the example above, some methods override the base
class in order to provide required functionality while others
just illustrate useful ways to divide tasks that should be
common to most bridges.

The first block of methods near the top of the declaration
are required in order for instances of the bridge to be
created and introspected by SMTK.

* The :smtk:`smtkDeclareModelingKernel` macro declares methods
  for introspection of the class.
  Bridge classes are managed by shared pointers and can be
  relatively heavyweight objects since they may contain maps
  from SMTK UUIDs to modeling kernel entities.

* The :cxx:`typedef smtk::shared_ptr<Bridge> Ptr` is required by some
  members declared in the :smtk:`smtkDeclareModelingKernel` macro.
  It is also useful for referencing shared pointers to the bridge
  internally.

* The :cxx:`typedef smtk::model::BridgedInfoBits BridgedInfoBits`
  is not required but will make implementing methods dealing with
  transcription of entities easier to type.

* The :cxx:`static BridgePtr create()` method is required in order
  for instances of the object to be created; its address is passed
  to the :smtk:`BridgeRegistrar` class by another macro discussed later
  so that instances of the bridge can be created given just a string
  describing the bridge.
  This is necessary so that bridges can be created and managed
  in remote processes.

* The virtual destructor should always be implemented so that the base
  class destructor is called.

* Finally, the :cxx:`allSupportedInformation` method exists so that SMTK
  can discover what types of information the bridge can provide to SMTK.
  The returned integer should be a bitwise OR of entries from
  the :smtk:`BridgedInformation` enum.
  For now, it is fine to return :smtk:`BRIDGE_EVERYTHING`.

The next step is to provide methods to access the maps between SMTK
and foreign entities.
The :cxx:`toEntity` and :cxx:`toCursor` methods do this and will
be discussed in more detail in the next section.
For our example, the :cxx:`EntityHandle` is defined like so:

.. literalinclude:: Bridge.h
   :start-after: // ++ 2 ++
   :end-before: // -- 2 --

Depending on your modeling kernel, you may use an existing type
from the foreign modeler (like the CGM bridge does) or a new class
like the one above (most useful for bridges discussed in
:ref:`bridge-by-sequence`).

The final public methods, :cxx:`staticSetup` and :cxx:`setup`, exist
so that applications can set global configuration parameters on modeling
kernels using a consistent API.
The :cxx:`staticSetup` method should be called before an instance of
the bridge class is constructed and may be used to perform one-time
initialization of the modeling kernel.
The :cxx:`setup` method is invoked on a particular instance of a bridge
and is used to set things such as the tolerances used to tessellate
curved geometry in CGM.
