Python bindings
---------------

Python scripts and managers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

New methods have been added to simplify scripting:

.. code-block:: python

   import smtk
   # Fetch a list of paths to SMTK plugins:
   pluginList = smtk.findAvailablePlugins()

   # Load a list of plugins:
   loaded, skipped = smtk.loadPlugins(pluginList)
   # If pluginList is omitted, all available plugins
   # will be loaded.

   # Fetch or create an application context:
   data = smtk.applicationContext()
   # (data will be an instance of smtk.common.Managers
   # initialized by calling all plugin registrars.)

These new methods work in both scripts and in ParaView-based
applications inside the interactive Python shell.
In the former case, the application context will be obtained
from the active client-server connection's SMTK wrapper object.
In the latter case, a :smtk:`smtk::common::Managers` object
will be created and initialized the first time the function is
called.

Python operations can now access SMTK managers and projects
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Python bindings were added so that Python operations can now retrieve the
``smtk::common::Managers`` object by calling ``smtk::operation::Operation::managers()``.
Individual manager instances can be retrieved from the managers object by calling
a new ``get()`` method and passing in the fully qualified class name of the object
to return. For example, to retrieve the project manager from the
``operateInternal()`` method:

.. code-block:: python

  project_manager = self.managers().get('smtk::project::Manager')


The ``smtk::project::Manager`` class was updated to add a ``projectsSet()`` method
so that Python operations can retrieve the projects for a given manager. The C++
methods returns a ``std::set<smtk::projectProject>`` and the Python binding returns
a Python set object.
