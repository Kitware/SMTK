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

Attribute-table views accept an optional ``AskToDelete`` boolean configuration
attribute that, if true, will use a modal popup dialog to confirm attribute deletions.
Otherwise, clicking the delete button will immediately delete all selected attributes.

When an attribute definition contains group items, either at the top level or
beneath a discrete item, selecting its table row displays a standard attribute
editor below the table. This makes group contents, including extensible groups,
editable while keeping the table and conditional-child columns synchronized.
The table and attribute editor are separated by a vertical splitter so users
can adjust how much space is allocated to each. The attribute-editor pane is
scrollable, allowing group contents to remain accessible at any splitter size.
The editor honors definition-specific inline ``ItemViews`` and named ``Style``
configuration on each ``AttributeTypes/Att`` entry, including configuration
inherited from base definitions.

The ``ColumnDisplay`` attribute controls which item columns are included:

* ``All`` (the default) includes every item supported by the table model.
* ``TopLevelNonGroup`` includes top-level items except group items.
* ``TopLevelDiscrete`` includes only top-level discrete items.
* ``UserSpecified`` includes the exact item paths listed under
  ``TableItems``.

For example:

.. code-block:: xml

   <View Type="AttributeTable" ColumnDisplay="UserSpecified">
     <AttributeTypes>
       <Att Type="BoundaryCondition"/>
     </AttributeTypes>
     <TableItems>
       <Item Path="name"/>
       <Item Path="boundaryType"/>
       <Item Path="boundaryType/temperature"/>
     </TableItems>
   </View>

``Path`` uses SMTK's slash-separated item-path syntax. ``Name`` is also
accepted as a convenience for top-level items. The attribute-name column is
always displayed.
