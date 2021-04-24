.. _smtk-project-sys:

---------------------
SMTK's Project System
---------------------

The project system provides a data management capability to assist
end users with creating, aggregating, and organizing SMTK resources
and operations used to carry out simulation tasks and workflows.
The Project System simplifies the loading and saving of a workflow
by treating all of the necessary resources and operations atomically.
In a non-project workflow, the user is responsible for loading all
of the resources (including the instantiation of attribute template
files) required to implement the workflow. The user is also responsible
for explicitly saving those resources and tracking them on disk.

In the case of a project workflow, a project is created via a template
or operation. Relevant attribute templates are instantiated automatically
when needed. Any resource required by the project but not automatically
generated is presented to the user as a requirement to be fulfilled or
assigned during project creation. Project data are stored in a
directory on the local file system. When a project is saved, all required
resources are saved atomically. Conversely loading a project also loads
in all of the project’s resources.

.. The SMTK project software includes operations to atomically load,
.. save, and close all SMTK resources in a project instance, along
.. with operations for registering project types, creating project
.. instances, and adding resources to project instances. The SMTK
.. Project System is an optional feature -- users can always manage
.. individual resources on a piecemeal basis.


.. toctree::
   :maxdepth: 3

   concepts.rst
