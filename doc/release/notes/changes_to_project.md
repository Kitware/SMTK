## Project Changes

### Refactored Project

`smtk::project` has been refactored to be more generic. Projects are now objects
that contain resources with assigned roles, and they scope the list of available
operations and resource types available to the project. Projects also
store/retrieve their contents by creating a portable .smtk file containing all of
the project's data. Projects can be described entirely using Python, making it
easier to describe custom project types for different solvers.
