## Introduce ParaView behavior that adds New Resource

Operations that share a common theme in the contexts of functionality
and graphical presentation are added to common operation groups. We
have constructed a new operation group for the creation of new
Resources. To utilize this group, we have also  added a file menu
option that allows a user to create a new Resource.

When there are multiple create operators registered to a single
Resource, the Resource action is listed as a menu that contains a list
of the individual create operations.

### User-facing changes

ParaView applications that load SMTK's pqAppComponents plugin now have
access to a menu action to construct a new SMTK resource, if an
operation is associated to that resource in the operation CreatorGroup.
