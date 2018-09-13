## ParaView-widget controls for SMTK attribute items

SMTK now makes some initial widgets (ParaView's box, plane, sphere,
and spline widgets) available for controlling values of items in
an SMTK attribute.
By adding an `ItemViews` section to an attribute's view and
setting the item's `Type` attribute to the proper string (one
of `Box`, `Plane`, `Sphere`, or `Spline`), the item will be
shown using a ParaView widget (and associated 3-d editor) when
the attribute is loaded into modelbuilder or paraview.
For example:

```xml
<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="Example">
      <ItemDefinitions>
        <Double Name="bbox" NumberOfRequiredValues="6">
        </Double>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="Demo" Type="Example">
      <Items>
        <Double Name="bbox">
          <Values>
            <Val Ith="0">0.0</Val>
            <Val Ith="1">0.05</Val>
            <Val Ith="2">0.0</Val>
            <Val Ith="3">0.05</Val>
            <Val Ith="4">0.0</Val>
            <Val Ith="5">0.125</Val>
          </Values>
        </Double>
      </Items>
    </Att>
  </Attributes>
  <Views>
    <View Type="Group" Title="Sample" TopLevel="true">
      <Views>
        <View Title="Example"/>
      </Views>
    </View>
    <View Type="Instanced" Title="Example">
      <InstancedAttributes>
        <Att Type="Example" Name="Demo">
          <ItemViews>
            <View Item="bbox" Type="Box"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
```

At this time, the widget⟷item connection is one-way; the item is updated
by the widget, but changes to the item outside of editing done using the
widget does not update the widget.
The latter cannot work until operations or some other signaling mechanism
exists to inform SMTK when changes are made to attribute item-values.

A limitation of the current implementation is that changes to the active
view do not affect which view the widget resides in.

In the longer term, this functionality may be expanded to other
ParaView widgets and provide a more flexible mapping between values
in SMTK items and their representative widget values.
For example, the XML above assumes the bounding box values
are stored as a single DoubleItem with 6 required values.
However, it is also common to think of a box as specified by
2 corner points (i.e., a GroupItem with 2 DoubleItem children)
or as a center point and a vector of lengths along each axis.
By accepting more options in the XML, we will allow a mapping
between the widget and items to be specified.

Finally, note that the box widget will only initialize its representation
with values from the SMTK item if they are non-default (or if
no default exists).
Similarly, the plane widget uses a default size for its widget which
is very inflexible.
We plan to extend this capability by accepting more XML attributes
in the AttributeItemInfo's Component entry that specify how to obtain
initial values from model components currently loaded into
modelbuilder/paraview.

### Developer changes

The new classes include:

+ `pqSMTKAttributeItemWidget` — a base class for all paraview-widget items; it inherits qtItem.
+ `pqSMTKBoxItemWidget` — a subclass of pqSMTKAttributeItemWidget that realizes a box widget.
+ `pqSMTKPlaneItemWidget` — a subclass of pqSMTKAttributeItemWidget that realizes a plane widget.
+ `pqSMTKSphereItemWidget` — a subclass of pqSMTKAttributeItemWidget that realizes a sphere widget.
+ `pqSMTKSplineItemWidget` — a subclass of pqSMTKAttributeItemWidget that realizes a spline widget.

The `pqSMTKAppComponentsAutoStart` class's initializer now
calls `qtSMTKUtilities::registerItemConstructor` to register
each concrete widget class above as a `qtItem` subclass.
Once the plugin is loaded, any attributes displayed in the
attribute panel or operation panel may request, e.g., the box
widget with `Type="Box"` as shown above.
