## Introduce ParaView behavior that imports files into an existing SMTK resource

We have added a file menu option that allows a user to select a data
file and import its contents into an existing resource. This differs
from the canonical File->Open method, which creates a new resource for
the imported data.

### User-facing changes

ParaView applications that load SMTK's pqAppComponents plugin now have
access to a menu action to import a data file into an existing resource.
