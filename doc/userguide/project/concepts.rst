Key Concepts
------------

The project system is composed of C++ classes,
also accessible in Python, whose instances perform the following functions:

:smtk:`Project`
  instances represent an encapsulation of a set of SMTK Resources and
  Operations for the purpose of accomplishing a targeted set of tasks. Each
  project instance contains Resources and a list of Operations that are
  pertinent to the Project. As a descendent of Resource, the Project class
  also contains links, properties, and Query functionality.
  When a project instance is read from the file system,
  all of its constituent resources are loaded as an atomic
  operation. Each resource in a project is identified by a
  unique string referred to as a *role*, which is specified
  when the resource is added to a project instance. The
  Project class provides accessors to the project resources,
  operations, and a project-version string.

:smtk:`Operation`
  is a base class for operations that require access to a project manager.
  Operations that inherit from this class and that are created by an operation
  manager that has a project manager observing it will have the project
  manager assigned to them upon creation. (Otherwise, the project manager must
  be set manually.)

:smtk:`Manager`
  is a singleton instance with methods for registering project types,
  registering project operations, creating projects, and accessing
  project instances. Project types are registered with four arguments:
  a unique name
  (string) identifiying the project type, an optional list of resource
  types that can be added to the project, an optional list of
  SMTK operations that can be applied to the project resources,
  and an optional version string (that defaults to "0.0.0").
  Each registered project type can either use the baseline SMTK
  Project class for storage or implement a subclass to include
  additional project data and/or behavior.
