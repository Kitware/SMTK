Python operations can now access SMTK managers and projects
------------------------------------------------------------

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
