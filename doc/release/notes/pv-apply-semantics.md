## Enforce ParaView semantics for apply() on File->Open only

When resoures are loaded via File->Open, ParaView semantics
apply: the apply button must be pressed to complete opening the
file. This matches the functionality of the other ParaView readers
(that are selectable when there are multiple readers for a file type).

When a resource is created via File->New Resource, a modal dialog
is presented to the user. When the user presses apply on the dialog,
the resource is generated and its representation is rendered. The
Properties Panel's apply button is never enabled.

When a resource is created by an operation from the Operation
Panel, the resource is generated and its representation is
rendered. The Properties Panel's apply button is never enabled.
