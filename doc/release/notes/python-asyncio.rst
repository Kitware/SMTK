Python Bindings
===============

SMTK's python bindings now accommodate running threaded C++ operations
in the background while notifying python scripts of results using the
``asyncio`` module's event loop.
See :ref:`python-asyncio` for more details.
