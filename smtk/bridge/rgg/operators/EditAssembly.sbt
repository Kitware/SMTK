<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "EditAssembly" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="edit assembly" Label="Model - Edit Assembly" BaseType="operator">
      <BriefDescription>Edit a RGG Assembly.</BriefDescription>
      <DetailedDescription>
        User can eidt an existing assembly by changing its properties. A schema planner
        is provided to plan the layout of nuclear pins in the assembly.
      </DetailedDescription>
      <AssociationsDef Name="assembly" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>aux_geom</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <ModelEntity Name= "associated duct" NumberOfRequiredValues="1">
          <MembershipMask>aux_geom</MembershipMask>
          <BriefDescription>A duct which is assocated with current assembly.</BriefDescription>
          <DetailedDescription>
            A duct which is assocated with current assembly. For now it would dicate assembly's
            size and height.
          </DetailedDescription>
        </ModelEntity>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(edit assembly)" BaseType="result">
      <ItemDefinitions>
        <!-- The editd assembly is returned in the base result's "edit" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkRGGEditAssemblyView ...)
      -->
    <View Type="smtkRGGEditAssemblyView" Title="Edit Assembly"  FilterByCategory="false"  FilterByAdvanceLevel="false" UseSelectionManager="false">
      <Description>
        TODO: Add documentation for edit assembly operator.
      </Description>
      <AttributeTypes>
        <Att Type="edit assembly"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeSystem>
