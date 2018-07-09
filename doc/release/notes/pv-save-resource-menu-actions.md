## Introduce ParaView behavior that adds Save Resource (As...)

Our goal is to have ModelBuilder be as minimially modified from stock
ParaView as possible. To this end, we have added a behavior to SMTK's
pqAppComponents plugin that inserts "Save Resource" and "Save Resource
As..." menu actions to the File menu.
The icon currently used for saving a resource is identical to
ParaView's icon for saving data. In the future we may want a unique
icon for reading/writing SMTK native resources.
These changes are an alternative to the changes in model builder
described
[here](https://gitlab.kitware.com/cmb/cmb/merge_requests/612); the
concerns identified therein are still applicable.

### User-facing changes

ParaView applications that load SMTK's pqAppComponents plugin now have
access to menu actions to save resources into SMTK's native (*.smtk) format.
