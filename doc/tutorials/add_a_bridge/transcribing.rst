***************************
Transcribing model entities
***************************

The main purpose of the bridge is to provide the SMTK model Manager
which owns it with information about the entities in some foreign modeler
and the :smtk:`Bridge::transcribeInternal` method is where your bridge
must do this.

The first thing you should do is verify that the entity being requested
actually exists:

.. literalinclude:: ../../../smtk/bridge/exodus/Bridge.cxx
   :start-after: // ++ 7 ++
   :end-before: // -- 7 --

One trick you'll see in most bridges is the construction of a "mutable" cursor
from the const version that passed to :cxx:`transcribeInternal`:

.. literalinclude:: ../../../smtk/bridge/exodus/Bridge.cxx
   :start-after: // ++ 8 ++
   :end-before: // -- 8 --

The const version does not provide access to methods that alter the model manager's storage.
Constructing a mutable version of the cursor is legitimate inside the bridge —
we are pretending that the entity to be transcribed has existed in the manager
ever since it existed in the foreign modeler.
Since transcription should not change the entity in the foreign modeler,
creating a mutable cursor is acceptable.

Once you know that the UUID has a matching entity in the foreign modeler,
you should create an :smtk:`Entity` instance in the model manager's map
from UUIDs to entities.
Make sure that the entity's flags properly define the type of the entity
at this point but don't worry about filling in the list of relations to
other entities unless the :smtk:`BRIDGE_ENTITY_RELATIONS` bit is set
in the request for transcription.

.. literalinclude:: ../../../smtk/bridge/exodus/Bridge.cxx
   :start-after: // ++ 9 ++
   :end-before: // -- 9 --

.. literalinclude:: ../../../smtk/bridge/exodus/Bridge.cxx
   :start-after: // ++ 10 ++
   :end-before: // -- 10 --

If :smtk:`BRIDGE_ENTITY_RELATIONS` is set, then you not only need to
ensure that the relations are listed but that they are also *at least*
partially transcribed.
You are always welcome to transcribe more than is requested,
but be aware that this can slow things down for large models.
Because this is an example and because the Exodus bridge does not
expose individual cells as first-class entities,
we transcribe the entire model recursively.

.. literalinclude:: ../../../smtk/bridge/exodus/Bridge.cxx
   :start-after: // ++ 11 ++
   :end-before: // -- 11 --

Now you should check other bits in :smtk:`BridgedInformation` that
are present in your :smtk:`Bridge::allSupportedInformation` method
and ensure that information is transcribed properly before returning
the bits which were actually transcribed for the given entity.

The :smtk:`Bridge::transcribe` method uses the return value to
update its list of dangling (partially transcribed) entities.
