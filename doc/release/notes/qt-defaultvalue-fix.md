## qtInputsItem

Extensible items that provided default values would cause a
crash once more entries were added than there were defaults.
This has been fixed to repeat the first default value for
all new entries that do not have a valid default.
