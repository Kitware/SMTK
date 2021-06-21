Added onDefinitionChange Signal to qtAttributeView
--------------------------------------------------

Added a signal called "onDefinitionChange" that will emitted when the user
selects a definition from the definition combo box. Classes that inherit
qtAttributeView can connect to this signal in order control the widget's
behavior based on the type of definition the user has selected.
