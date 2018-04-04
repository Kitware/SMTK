<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "RemoveMaterial" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="remove material" Label="Model - Remove Material" BaseType="operator">
      <BriefDescription>
        Remove a user defined material.
      </BriefDescription>
      <DetailedDescription>
        Remove a user defined material.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
            <String Name="name" NumberOfRequiredValues="1" AdvanceLevel="11">
              <BriefDescription>A user assigned name for the material</BriefDescription>
            </String>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(remove material)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkRGGRemoveMaterialView ...)
      -->
    <View Type="smtkRGGRemoveMaterialView" Title="Remove Material"  FilterByCategory="false"  FilterByAdvanceLevel="false" UseSelectionManager="false">
      <Description>
        Remove a material definition from this RGG model.
      </Description>
      <AttributeTypes>
        <Att Type="remove material"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeSystem>
