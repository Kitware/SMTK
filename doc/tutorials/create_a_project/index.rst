Create a basic project
======================

.. highlight:: c++
.. role:: cxx(code)
   :language: c++

This example describes how to create a basic SMTK project, add
an attribute resource to it, and write the project to disk.
A complete/working source file is included in the SMTK source repository,
as file ``smtk/doc/tutorials/create_a_project/create_a_project.cxx``.
Snippets from that source file are described below.

Initialize SMTK Managers
------------------------

To use SMTK projects, you first create SMTK resource manager,
operation manager, and project manager instances, and then
register the various SMTK features. In this example, we will be
creating an attribute resource, so we also register the SMTK
``attribute`` feature to the resource and operation managers.

.. literalinclude:: create_a_project.cxx
   :start-after: // ++ 1 ++
   :end-before: // -- 1 --
   :linenos:


Register and Create "basic" Project
-----------------------------------

SMTK projects are registered by a type (string) that is registered
to the project manager. In the simplest case, you can register a project
type with just a string, but you can also register the string plus a
subclass of ``smtk::project::Project`` for more advanced applications.
Although any project type can be used, for this example we deliberately
chose to register "basic" because the CMB modelbuilder application also
registers that type by default. As a result, the project saved in this
example can be loaded into modelbuilder. Other project types require
updating modelbuilder, typically via an SMTK plugin, to register the
project type.


.. literalinclude:: create_a_project.cxx
   :start-after: // ++ 2 ++
   :end-before: // -- 2 --
   :linenos:

Create Attribute Resource
-------------------------

Next we create a simple attribute resource for the project contents.
Standard practice would use the SMTK ImportResource or ReadResource
operations to load attribute and other resources, but in order to
implement this example as a single source file, an inline string is
used for the attribute template. Because the template is manually
generated, we also check the error flag returned when the string is
read.

In the last line of this snippet, we also create a single attribute
instance to display in the instanced view. Refer to the attribute
resource documentation for more information about create attribute
templates and instances.

.. literalinclude:: create_a_project.cxx
   :start-after: // ++ 3 ++
   :end-before: // -- 3 --
   :linenos:


Add Attribute Resource to Project
---------------------------------

Adding a resource to a project is a single API call, passing in then
resource (shared pointer) and a string identified called "role". For
example, a casting simulation project might have mulitple attribute
resources with roles such as "heatup specification", "pour",
"solidifcation", and mesh resources with roles such as "heat transfer mesh",
"induction heating mesh", and "fluid flow mesh".

.. literalinclude:: create_a_project.cxx
  :start-after: // ++ 4 ++
  :end-before: // -- 4 --
  :linenos:

Write Project Resource
----------------------

Because the SMTK project class is a subclass of the SMTK resource class,
The standard SMTK WriteResource operation can be used to serialize the
project to the file system. To do this, specify a project filename
with the standard ".smtk" extension. In this example, the operation
writes 2 files to disk, the specified project file (the filename
``basic-project.smtk`` is used in the code snippet) and a separate file
for the attribute resource that was added to the project. Project
resource files are written to a ``resources`` subfolder. The filename
used for the attribute resource file is its rolename plus the ".smtk"
extension, in this case, ``attributes.smtk``. So for this example,
the outputs to the filesystem are:

::

  working directory
  |-- basic-project.smtk
  |-- resources/
  |---- attributes.smtk

The code to do this is the same as writing any SMTK resource.

.. literalinclude:: create_a_project.cxx
   :start-after: // ++ 5 ++
   :end-before: // -- 5 --
   :linenos:


Open in modelbuilder Application
--------------------------------

As noted above, the CMB modelbuilder application is configured by default
to register SMTK projects of type "basic". So you can use the modelbuilder
``File => Open`` menu to load the ``basic-project.smtk`` file which loads
the entire project which, in this case, is just the single attribute
resource. When you do this, enable the "Attribute Editor" and "Projects"
views. An example the resulting display is show here. The "Projects" view,
in the lower part, shows a tree view of the projects currently loaded, in
this case, our basic project example. Above that is the "Attribute Editor"
for the simple attribute resource we created and included in the project.

.. figure:: project-ui.png
   :align: center

Going Further
-------------

For extra credit, you can obtain this source file at
``smtk/doc/tutorials/create_a_project/create_a_project.cxx`` and
continue adding features, for example:

* Replace the inline attribute with an external .sbt file.
* Use the SMTK ReadResource operation to create the attribute
  resource from the .sbt file.
* Load a model file and add that to the project.
* Extend the attribute template to include model associations.
