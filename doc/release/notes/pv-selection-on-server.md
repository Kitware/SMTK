## Move ParaView selection logic to the server

A change has been made to the logic that converts ParaView selections
to SMTK selections. Originally, this selection logic occurred using a
Qt signal/slot that subsequently pierced client/server to access
rendered data. With this update, the logic has been moved to the
server to avoid additional client/server transgressions.
