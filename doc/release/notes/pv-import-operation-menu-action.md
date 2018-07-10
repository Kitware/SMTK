## Introduce ParaView behavior that imports smtk python operation

We have added a file menu option that allows a user to select a python
file describing an SMTK operation and import this operation into a
server's operation manager.

### User-facing changes

ParaView applications that load SMTK's pqAppComponents plugin now have
access to a menu action to import a python-based SMTK operation.
