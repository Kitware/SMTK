.. _smtk-tips:

-------------------------
Debugging tips and tricks
-------------------------

Several design patterns in SMTK can complicate debugging:

* SMTK allows asynchronous operations run in separate threads.
* Shared pointers mean that resources and components may sometimes
  not be destroyed when you expect.
* The included ParaView extensions have to deal with potentially
  separate client and server processes.

This section provides some tips on debugging to help mitigate the complexity.

.. toctree::
   :maxdepth: 3

   observers.rst
