## Changes to the View System and Qt Extensions

### Changes to qtBaseView

* The qtBaseView class has been split into qtBaseView and qtBaseAttributeView.
  All of the existing qtBaseView subclasses now instead inherit qtBaseAttributeView.
* The displayItem test now calls 2 new methods categoryTest and advanceLevelTest.  This makes it easier for derived classes to override the filtering behavior

### Changes to displaying double items
Using ItemViews you can now control how the double value item is displayed based using the following "attributes":

* Notation - general display behavior.  Supported values include:
 * Fixed - displays the value in fixed notation.  This is equivalent to printf's %f flag
 * Scientific - displays the value in scientific notation.  This is equivalent to printf's %e flag
 * Mixed - tries to determine the best notation to use.  This is equivalent to printf's %g flag
* Precision - controls the precision (in the case of Fixed and Scientific Notations) or significant digits (in the case of Mixed Notation) that are to be displayed when the value is not being edited.
* EditPrecision - controls the precision (in the case of Fixed and Scientific Notations) or significant digits (in the case of Mixed Notation) that are to be displayed when the value is being edited.

Example SBT Code:

```xml
    <View Type="Instanced" Title="General">
      <InstancedAttributes>
        <Att Name="numerics-att" Type="numerics">
          <ItemViews>
            <View Item="dt_init" Type="Default" Precision="6" EditPrecision="10"/>
            <View Item="dt_max" Type="Default" Precision="6" EditPrecision="10" Notation="Fixed"/>
            <View Item="dt_min" Type="Default" Precision="6" EditPrecision="10" Notation="Scientific"/>
          </ItemViews>
        </Att>
        <Att Name="outputs-att" Type="outputs" />
        <Att Name="simulation-control-att" Type="simulation-control" />
<!--         <Att Name="Mesh" Type="mesh" /> -->
      </InstancedAttributes>
    </View>
```
See [SMTK Issue 270 to see what the resulting UI looks like.](https://gitlab.kitware.com/cmb/smtk/issues/270)

### Other Changes
* qtItem::updateItemData has been made public so that qtItems can be undated when their underlying attribute items are external changed.
