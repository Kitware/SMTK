.. _python-overview:

Python overview
===============

There are several python environments from which you can use SMTK

+ Interactively inside ParaView's (or ModelBuilder's) Python Shell.
  In this case, many plugins have already been loaded and you can
  obtain pre-existing managers from
  ``smtk.extension.paraview.appcomponents.pqSMTKBehavior.instance()``.
  Also, python commands you run may have an effect on the user interface
  of the application.
  You are responsible for importing whatever SMTK modules you need.
+ Interactively from a python command prompt or by running a python script.
  In this case, plugins are not loaded unless you manually load them
  (discussed below).
  You are responsible for importing whatever SMTK modules you need.
  User interface components are not available.
+ From an SMTK operation written in python.
  In this case, you can assume the environment is prepared, either by
  the script or the user interface.
  However, operations should generally not attempt to perform user
  interaction or assume a user interface is present.
  You are responsible for importing whatever SMTK modules you need.

Regardless of the environment, SMTK provides python support via two mechanisms:

+ python modules (``smtk``, ``smtk.resource``, ``smtk.operation``, …) that
  you can import and
+ shared-library plugins (built when ParaView support is enabled) that you can load.

Python modules provide access to C++ classes.
The modules are arranged to mirror the directory structure of SMTK's source
code (e.g., the ``smtk.resource`` module contains bindings for C++ classes
in the ``smtk/resource`` directory).
C++ classes are wrapped as needed and more effort has been put into wrapping
classes that expose basic functionality than into subclasses that extend
functionality.
This is because most of SMTK's functionality can be exercised via the
methods on base classes such as :smtk:`smtk::resource::Component`;
frequently subclasses do not need wrapping.

Second, shared-library plugins can be loaded from python.
These plugins are typically used to register resource types,
operations, and view classes to managers.
While it is possible to wrap the ``Registrar`` classes each
subsystem of SMTK provides, this is not always done.
In these cases, you should load the plugin and call
the ``smtk.plugin.registerTo()`` method to populate your Manager
instances with classes contained in the loaded plugins.

Consider the following python script:

.. literalinclude:: loadPlugin.py
   :start-after: # ++ 1 ++
   :end-before: # -- 1 --
   :linenos:

While it imports the operation and resource modules and creates managers,
these managers are not initialized with any resource types or operations
because no registrars have been added to the plugin registry.
We can load plugins like so:

.. literalinclude:: loadPlugin.py
   :start-after: # ++ 2 ++
   :end-before: # -- 2 --
   :linenos:

With the plugins loaded, the registrars have been added and the
managers can be registered to all the loaded plugins.
Finally, we can then ask the resource manager to load a resource
for us:

.. literalinclude:: loadPlugin.py
   :start-after: # ++ 3 ++
   :end-before: # -- 3 --
   :linenos:

As an alternative, we can create an operation and run it
to load or import a file.
The example below imports an SimBuilder Template (SBT) file.

.. literalinclude:: loadPlugin.py
   :start-after: # ++ 4 ++
   :end-before: # -- 4 --
   :linenos:


.. _smtk-python-plugin:

Python plugins in modelbuilder
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Because of the complex mechanics of the different python environments,
modelbuilder provides a simple way to register operations that should
be automatically available each time you run (rather than requiring
you to manually click on the "File→Import Operation…" menu each time).

Instead, once you have your Python operation defined in a module file,
you can tell modelbuilder to treat it as a plugin, which can be set to
auto-load on startup.
To do this, we'll add a few lines of code to the bottom of your module
like so:

.. code-block:: python

   import smtk.operation

   class CustomOperation(smtk.operation.Operation):
       # Define your operation methods here as usual...

   if __name__ != '__main__':
       from contextlib import suppress
       with suppress(ModuleNotFoundError):
           import smtk.extension.paraview.appcomponents as app
           app.importPythonOperation(__name__, 'CustomOperation')


In the example above, you will already have defined the ``CustomOperation``
class and only need to add the ``if``-block at the bottom.
If your module has multiple operations, you can call ``app.importPythonOperation()``
as many times as you like.

Once you have added this to your python module, click on
the "Tools→Manage Plugins…" menu item in modelbuilder.
When the dialog appears, click on "Load New" and select
your module. It should load and immediately register your
new operation. If you want the operation to be available
each time you start modelbuilder, just click on the "Auto-load"
option in the plugin manager and exit modelbuilder;
the setting will be saved and your module will be imported
on subsequent runs.
