.. highlight:: c++
.. role:: cxx(code)
   :language: c++

.. role:: cmake(code)
   :language: cmake

.. _subclassing-session:

***************************
Creating a session subclass
***************************

Sessions exist to link foreign modeling entities to SMTK
modeling entities, in a bidirectional way:

* we *transcribe* foreign modeling entities into an SMTK model resource, and
* we perform *operations* in SMTK that make changes in the foreign modeling
  kernel (and then result in more transcriptions to update SMTK's model resource).

Only the first of these is needed for read-only access so we will cover it
first and then describe the interactions between sessions and operators.
:ref:`Implementing operators <tut - implement an operator>` is the topic
of a separate tutorial.

The first thing you must do when creating your own session is
to implement a subclass of :smtk:`smtk::model::Session`:

.. literalinclude:: ../../../smtk/session/exodus/Session.h
   :start-after: // ++ 1 ++
   :end-before: // -- 1 --

In the example above, some methods override the base
class in order to provide required functionality while others
just illustrate useful ways to divide tasks that should be
common to most session types.

The first block of methods near the top of the declaration
are required in order for instances of the session to be
created and introspected by SMTK.

* The :cxx:`typedef smtk::shared_ptr<Session> Ptr` is useful for
  referencing shared pointers to the session internally.

* The :cxx:`typedef smtk::model::SessionInfoBits SessionInfoBits`
  is not required but will make implementing methods dealing with
  transcription of entities easier to type.

* The :cxx:`smtkCreateMacro` macro is required in order
  for instances of the object to be created.

* The virtual destructor should always be implemented so that the base
  class destructor is called.

* Finally, the :cxx:`allSupportedInformation` method exists so that SMTK
  can discover what types of information the session can provide to SMTK.
  The returned integer should be a bitwise OR of entries from
  the :smtk:`SessionInformation` enum.
  For now, it is fine to return :smtk:`SESSION_EVERYTHING`.

The next step is to provide methods to access the maps between SMTK
and foreign entities (in this case, Exodus element blocks, side sets,
and node sets).
The :cxx:`toEntity` and :cxx:`toEntityRef` methods do this and will
be discussed in more detail in the next section.
Depending on your modeling kernel, you may use an existing type
from the foreign modeler (like the CGM session does) or a new class
like the :cxx:`EntityHandle` class in our example.

Now that we have defined a mapping between UUIDs
and model entities, the next step is to have the
session transcribe information about foreign model
entities into a :smtk:`model resource <smtk::model::Resource>`
instance.
