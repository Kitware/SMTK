Allow qtOperationDialog to be non-modal
---------------------------------------

If the qtOperationDialog is shown non-modal, using `show()` instead of `exec()`,
the Apply button doesn't close the dialog, but instead will remain active and
allow the operation to be run again.
