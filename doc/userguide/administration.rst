.. role:: cxx(code)
   :language: c++

.. role:: arg(code)
   :language: sh

.. _smtk-administration:

******************
Administering SMTK
******************

Previous sections covered the concepts and tools for using SMTK.
This section is for system administrators who wish to make SMTK
available to users

* as a Python module and command-line utilities for end users of SMTK,
* as a library for people developing SMTK-based applications, and/or
* as a remote model and mesh server for end users and applications.

End-user tool installation
==========================

This type of installation should be as simple as downloading a
binary package of SMTK and clicking install.

.. todo:: Expand on details of installation and configuration.

Developer installation
======================

In addition to the binary installer, there should also be a development
package that contains header and configuration files needed to build
C++ applications using SMTK. Install this the same way you installed
the binary above.

You can also download the source code from the git repostory and
follow the instructions for building and installing SMTK in the
toplevel :file:`ReadMe.mkd` file.

.. todo:: Expand on details of installation and configuration.

Configuration as a modeling and meshing server
==============================================

SMTK uses Remus_ to start model and mesh workers.
In order for SMTK to discover available workers,
you must place Remus worker files somewhere that SMTK
is configured to search for them.
These worker files identify the executable to
run when a user requests a modeling or meshing operation
to be performed.
Their format is covered further below, but first we
focus on how the files are discovered.

Worker file search paths
------------------------

The default locations that SMTK searches for these
worker files varies by operating system:

Linux
    SMTK searches the current working directory of the
    process, followed by the :file:`var/smtk/workers` subdirectory
    of the toplevel installation directory.
    For example, if SMTK is installed into :file:`/usr`
    with the worker at :file:`/usr/bin/smtk-model-worker`,
    then it will search :file:`/usr/var/smtk/workers`.

    If the :cxx:`SMTK_WORKER_DIR` environment variable is set
    to a valid path, then it is searched as well.

Mac OS X
    SMTK searches the current working directory of the
    process, followed by the :file:`var/workers` subdirectory
    of the toplevel installation directory if SMTK is not part of a bundle.
    For example, if SMTK is installed into :file:`/usr`
    with the worker at :file:`/usr/bin/smtk-model-worker`,
    then it will search :file:`/usr/var/smtk/workers`.

    If an application built with SMTK is part of a bundle (such as an app),
    then SMTK will search the :file:`Contents/Resources/workers` directory
    of the bundle.

    If the :cxx:`SMTK_WORKER_DIR` environment variable is set
    to a valid path, then it is searched as well.

Windows
    SMTK searches the current working directory of the process
    followed by the directory containing the process executable
    (when provided to SMTK by the application).

    If the :cxx:`SMTK_WORKER_DIR` environment variable is set
    to a valid path, then it is searched as well.

.. _Remus: https://github.com/robertmaynard/Remus

Creating a Remus worker file for solid modeling
-----------------------------------------------

When SMTK is built with Remus support enabled, it will include a
command-line utility named :file:`smtk-model-worker`.
This program can be run manually or directly by SMTK in order
to perform modeling operations in a different process.
It is also the program you can run to generate a worker file
that makes it discoverable to SMTK for direct use.
You can run

.. code:: sh

  smtk-model-worker -help

to obtain reference information on the command-line arguments.
It will also print a list of available modeling kernels.

Each model worker exposes a single modeling kernel (via the
:smtk:`RemusRemoteSession` on the client, which talks to
a :smtk:`RemusRPCWorker` in the worker process).
Normally, the model worker executable expects to be given the
following command-line arguments:

* A Remus server to connect to as its first argument, formatted
  as a URL (e.g., :arg:`tcp://cadserver.kitware.com:50510`).
* A solid modeling kernel to advertise (e.g., :arg:`-kernel=cgm`).
* A default modeling engine for the kernel to use (e.g., :arg:`-engine=OpenCascade`).
* A Remus worker file to read (when invoked without :arg:`-generate`) or
  write (when invoked with :arg:`-generate`).
* A directory in the filesystem to make available to users for reading and
  writing CAD model files (e.g., :arg:`-root=/usr/local/var/smtk/data`).
  The root directory is not made available to end users for security
  purposes; all paths are relative to the root directory.
  SMTK does not currently prevent access to other portions of the
  filesystem but it will in the future.
* A "site name" describing the combination of host and/or filesystem
  made available to SMTK by the model worker. This label is presented
  to end users by applications so that users can differentiate between
  workers providing the same modeling kernels but on different machines.
* A Remus worker file to read or write
  (e.g., :arg:`-rwfile=/usr/local/var/smtk/workers/cgm-OCC.rw`).

If you pass the :arg:`-generate` option to the model worker,
then it will generate a model worker file which you can then
customize.
When you generate a model worker file,
two files are normally written:
the first, specified by the :arg:`-rwfile` argument is the
actual Remus worker file and is formatted as a JSON object.
The second has the same filename with a :file:`.requirements`
suffix appended and is formatted as an XML attribute system
describing the modeling operations available.

You should generate a separate Remus worker file for each combination
of modeling kernel, engine, and root directory you wish to make
available.
Once these files have been generated,
you can edit the worker file and change them to suit your site's needs.
You may specifically wish to change the WorkerName setting to be more
descriptive.
Be careful when editing the Tag data as it is used by SMTK
to decide which engine and kernel combination may load a given
file.
