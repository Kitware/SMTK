Qt UI Changes
===================

Added UI Mechanism to Aid Locating Expressions
-----------------------------------------------

With the expansion of the expression system in Value Items, an optional filtering mechanism was added to
the expression frame view in qtInputsItem.

A QPushButton was adding to display a QLineEdit, whose text is used as a regular expression to filter the options in the
expression QComboBox. A custom QSortFilterProxyModel is used such that applying any filter will always include the current expression
and the default "Please Select" or optional "Create..." options.

Each item within the QComboBox now has a tool tip showing the expression's type.
