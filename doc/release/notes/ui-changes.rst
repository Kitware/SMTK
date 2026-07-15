QT Changes
===========

qtComponentAttributeView
--------------------------

If there is only one Attribute Definition available then the view
will automatically create Attributes for each Resource Component that
are applicable and will not display a combo-box in the second column and
instead displays the sole Definition's type.

Displaying Attributes in Tabular Format
---------------------------------------
Added the following classes:

* qtAttributeTableView - a View that manages attributes of a specified type using a table-based approach
* qtAttributeTableModel - a Qt Model for attributes of a given type used by the above View
* qtAttributeTableDelegate - a Qt delegate used by the above classes

Attribute-table views accept an optional ``HideInactiveChildren`` boolean
configuration attribute. When enabled, selecting a row hides columns for
conditional children that are inactive for that row. The columns are updated
when a discrete value is edited and are restored when no row is selected.
