Made projects portable between different file systems and paths
---------------------------------------------------------------

Changed absolute file paths used in the project relative paths where
necessary to allow the project folder to moved to a new location, or
even another machine.

Developer changes
~~~~~~~~~~~~~~~~~~

No new absolute paths should be added to the project. Any new paths that
are saved should be saved as relative to the project main folder.  Ideally,
all files and folders that are part of a project should be contained
within the main project folder.

User-facing changes
~~~~~~~~~~~~~~~~~~~

With this change in plase, projects can be freely moved on a given machine,
or shared to a completely new machine in a seemless fashion.
