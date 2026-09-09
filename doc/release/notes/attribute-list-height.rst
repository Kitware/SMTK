Compact attribute lists
-----------------------

Attribute views now prefer an initial list height of two rows, leaving more space
for editing the selected attribute. Set ``AttributeListRows`` on the view to
choose a different initial height, for example::

  <View Type="Attribute" Title="Boundary Conditions" AttributeListRows="4">
    <AttributeTypes>
      <Att Type="BoundaryCondition"/>
    </AttributeTypes>
  </View>

The splitter remains adjustable. Invalid or non-positive values fall back to two
rows.
