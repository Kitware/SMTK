Optional progress bar in qtOperationDialog
==========================================

A QProgressBar has been added to qtOperationDialog. It is hidden (visibility false) by default. To use this feature, call `qtOperationDialog::progressBar()`, which returns a pointer to the QProgressBar instance. When the visibility is on, the widget appears in the lower left of the dialog, to the left of the Apply and Cancel buttons.
