<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "AssignColors" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="assign colors" Label="Assign Colors" BaseType="operation">
      <AssociationsDef Name="entities" NumberOfRequiredValues="1" Extensible="true">
        <Accepts>
          <Resource Name="smtk::resource::Resource" Filter="*"/>
          <Resource Name="smtk::resource::Resource"/>
        </Accepts>
      </AssociationsDef>
      <BriefDescription>
        Assign one or more colors to one or more entities using a color palette.
      </BriefDescription>
      <DetailedDescription>
        Assign colors from the given list to the associated
        model entities in a round-robin fashion.

        This operator will also remove colors from the associated
        model entities if the list of colors to assign is empty.
      </DetailedDescription>
      <ItemDefinitions>
        <String Name="colors" NumberOfRequiredValues="0" Extensible="true" Optional="true" IsEnabledByDefault="true">
          <BriefDescription>A list of colors to assign to the associated entities.</BriefDescription>
          <DetailedDescription>
            Each color in the list is either a canonical color name or
            a hexadecimal RGB string beginning with '#'
            (e.g., "#ff0000" or "#f00" for pure, fully-saturated red).

            An empty list indicates that the color property should be removed
            from each entity, but only if the item is enabled.
            If the item is disabled and the list is empty, there is no effect.
          </DetailedDescription>
        </String>
        <Double Name="opacity" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false" AdvanceLevel="13">
          <BriefDescription>An opacity to assign to the associated entities.</BriefDescription>
          <DefaultValue>1.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0.0</Min>
            <Max Inclusive="true">1.0</Max>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(assign colors)" BaseType="result"/>
  </Definitions>

  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkAssignColorsView ...)
      -->
    <View Type="smtkAssignColorsView" Title="Choose Colors"  FilterByCategory="false"  FilterByAdvanceLevel="false" UseSelectionManager="true">
      <Description>
        Click either the single-color swatch or the palette
        swatch to assign a color to each associated entity.
        If you click on the single-color swatch, every entity will get that color.
        If you click on the palette swatch, every entity will get a different color
        from the palette. When there are more entities than palette colors, some entities
        will get the same palette color.

        You can also click on the swatch with the slash through it to remove any
        previously-assigned color from the associated entities. For now it would not
        remove the color assigned by entity list(In order to do so, when assigning a color
        to entity list you should just click Cancel button in the dialog).

        You can pick a new color for the single-color swatch or a new palette for
        the palette swatch by clicking the buttons below the swatches, respectively.
        Once you choose a new value, it is immediately applied to all the associated
        entities.
      </Description>
      <AttributeTypes>
        <Att Type="assign colors"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
