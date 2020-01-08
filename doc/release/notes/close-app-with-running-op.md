## Behavior for intercepting application close requests when operations are running

A behavior has been added that tracks when operations are active. When a close
event is received from the main window, a modal dialog confirming the
application's close is presented if there are active operations.

### User-facing changes

When an operation is running, applications built on the ParaView framework will
now warn and provide an option to cancel close events.
