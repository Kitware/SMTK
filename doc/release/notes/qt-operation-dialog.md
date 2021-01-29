## Operation dialog

A qtOperationDialog class is addded for launching SMTK operations from a
modal dialog. The intended use is for modelbuilder plugins that use menu or
similar actions to invoke SMTK operations. (For example, export operations
are typically run from a modal dialog.) The qtOperationDialog is created
as a QTabWidget containing 2 tabs. The first tab embeds a qtOperationView
for the SMTK operation, and the second tab displays the operation's "info"
as formatted by qtViewInfoDialog.
