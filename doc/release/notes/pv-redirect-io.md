## Redirect singleton smtk::io::Logger to output to ParaView output window

The singleton smtk::io::Logger instance now pipes its records to Qt's
messaging system, which is intercepted by ParaView and displayed in
ParaView's output window.

### User-facing changes

ParaView applications that load SMTK's pqAppComponents plugin now can
read any SMTK messages using ParaView's output window.
