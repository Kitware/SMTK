## SMTK Project Manager

New SMTK classes are added for organizing resources into simulation projects,
as outlined in a [discourse topic](https://discourse.kitware.com/t/new-modelbuilder-plugin-for-projects/176) on 14-Nov-2018.
The primary API is provided by a new smtk::project::Manager class, with
methods to create, save, open, and close projects. The resources contained
by a project are represented by a new smtk::project::Project class.
For persistent storage, the resources contained a project are stored in a
user-specified directory on the available filesystem.
