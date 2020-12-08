## Done button for operation editor

The qtOperationView widget now includes a "Done" button.
Clicking it emits a signal that the parent pqSMTKOperationPanel
uses to delete the qtOperationView and its widgets.
