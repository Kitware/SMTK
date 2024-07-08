Changes to SMTK Common
======================

Removing Default Role Type for Links
------------------------------------

Links will no longer have a default role type of undefinedRole since this lead to issues where the specified role was not being used.
This role type has been removed.

Resource based Links now use the same value to represent an Link with an invalid role type.
Also by default, links with invalid role types  will no longer be copied when a resource is copied.
