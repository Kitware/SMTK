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

* we *transcribe* foreign modeling entities into an SMTK model manager, and
* we perform *operations* in SMTK that make changes in the foreign modeling
  kernel (and then result in more transcriptions to update SMTK's model manager).

Only the first of these is needed for read-only access so we will cover it
first and then describe the interactions between sessions and operators.
:ref:`Implementing operators <tut - implement an operator>` is the topic
of a separate tutorial.

The first thing you must do when creating your own session is
to implement a subclass of :smtk:`smtk::model::Session`:

.. literalinclude:: ../../../smtk/bridge/exodus/Session.h
   :start-after: // ++ 1 ++
   :end-before: // -- 1 --

In the example above, some methods override the base
class in order to provide required functionality while others
just illustrate useful ways to divide tasks that should be
common to most session types.

The first block of methods near the top of the declaration
are required in order for instances of the session to be
created and introspected by SMTK.

* The :smtk:`smtkDeclareModelingKernel` macro declares methods
  for introspection of the class.
  Session classes are managed by shared pointers and can be
  relatively heavyweight objects since they may contain maps
  from SMTK UUIDs to modeling kernel entities.

* The :cxx:`typedef smtk::shared_ptr<Session> Ptr` is required by some
  members declared in the :smtk:`smtkDeclareModelingKernel` macro.
  It is also useful for referencing shared pointers to the session
  internally.

* The :cxx:`typedef smtk::model::SessionInfoBits SessionInfoBits`
  is not required but will make implementing methods dealing with
  transcription of entities easier to type.

* The :cxx:`static SessionPtr create()` method is required in order
  for instances of the object to be created; its address is passed
  to the :smtk:`SessionRegistrar` class by another macro discussed later
  so that instances of the session can be created given just a string
  describing the session.
  This is necessary so that sessions can be created and managed
  in remote processes.

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

The final public methods, :cxx:`staticSetup` and :cxx:`setup`, exist
so that applications can set global configuration parameters on modeling
kernels using a consistent API.
The :cxx:`staticSetup` method should be called before an instance of
the session class is constructed and may be used to perform one-time
initialization of the modeling kernel.
The :cxx:`setup` method is invoked on a particular instance of a session
and is used to set things such as the tolerances used to tessellate
curved geometry in CGM.

Static initialization
---------------------

Now that we have declared the Exodus session class methods we must implement them.
In the :file:`Session.cxx` file, you will see that the :smtk:`smtkDeclareModelingKernel`
has a partner macro named :smtk:`smtkImplementsModelingKernel`, placed at the bottom
of the file **outside of any namespaces**:

.. literalinclude:: ../../../smtk/bridge/exodus/Session.cxx
   :start-after: // ++ 1 ++
   :end-before: // -- 1 --

This macro takes 6 parameters:

1. An export macro (or empty argument) specifying a preprocessor macro used to
   make the session accessible outside of the library it is linked to.
   The export macro is normally declared in your :file:`CMakeLists.txt` file
   by the :cmake:`smtk_export_header` function.

2. A "short" name for the session. This is used as part of some variable names inside the macro,
   so you should not use punctuation other than underscores.
   The short name ("exodus") will be used as the string name for the session and
   in other macros like :smtk:`smtkComponentInitMacro` when you name the session
   as a component to be initialized at link-time.

3. A string containing a valid JSON dictionary describing the capabilities of the session.
   We pass the argument :cxx:`Session_json`, which is defined in the :file:`Session_json.h`
   header file.
   The header is generated by the CMake :cmake:`smtk_session_json` macro,
   which simply encodes the contents of a JSON file as a C string for your convenience.
   The Exodus session has the following description:

   .. literalinclude:: ../../../smtk/bridge/exodus/Session.json

   At a minimum, the JSON dictionary must include

   + The "kernel" entry set to the name of the session.
     The kernel name should match the "short" name passed to :smtk:`smtkImplementsModelingKernel`.
   + A list of modeling engines that the kernel supports and the capabilities of each.
     If your session only supports a single modeling engine, use the name "default" as the Exodus session does.
     At a minimum, the capabilities for each engine should include a list of
     file extensions that the session can read.
     The list of standard capabilities will be expanded in in the future,
     but even now can be used by sessions in an ad-hoc manner.

4. An :cxx:`smtk::function` to be invoked with any configuration parameters
   before an instance of the session is created.
   The function should take two arguments:
   a :smtk:`String` parameter name and
   a :smtk:`StringList` parameter value.
   If you do not have any static configuration parameters, then
   simply pass :smtk:`SessionHasNoStaticSetup`.
   The CGM session provides an implementation of this to set the default
   modeling kernel engine (e.g., OpenCascade, FACET) since the
   engine should be prepared before the session is constructed.
   Otherwise, the modeling operations listed in the session's
   attribute resource might not reflect those available for the engine
   being used.

5. The fully-qualified name of the session class, including namespaces.

6. Either :cxx:`true` or :cxx:`false`, used to indicate whether the
   session should inherit operators from its subclass.
   You should pass true unless your session is a "forwarding" session
   (i.e., one that forwards operations to a remote process rather than
   performing them locally).

Session constructor
-------------------

Besides the macro declaration, your constructor **must** provide
the base class with the place it stores information
about session-specific operators:

.. literalinclude:: ../../../smtk/bridge/exodus/Session.cxx
   :start-after: // ++ 2 ++
   :end-before: // -- 2 --

The :smtk:`Session::initializeOperatorSystem` method creates
a new attribute :smtk:`Resource` and populates it with all the
operators in the given :cxx:`Session::s_operators` member,
which is declared by the :cxx:`smtkDeclareModelingKernel` macro
and instantiated by the :cxx:`smtkImplementsModelingKernel` macro.
This allows session-specific operators to be statically initialized and
registered at link-time.
The :cxx:`Session::s_operators` member is populated with operators by calls to
the :smtk:`smtkImplementsModelOperator` macro inside each operator's
implementation.

Now that we have defined a mapping between UUIDs
and model entities, the next step is to have the
session transcribe information about foreign model
entities into a :smtk:`model manager <smtk::model::Manager>`
instance.
