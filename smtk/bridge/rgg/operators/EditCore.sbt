<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "EditCore" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="edit core" Label="Core - Edit" BaseType="operator">
      <BriefDescription>Edit a RGG nuclear core.</BriefDescription>
      <DetailedDescription>
        Edit a RGG nuclear core's properties.
      </DetailedDescription>
      <AssociationsDef Name="Core" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>group</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="name" NumberOfValuesRequired="1" AdvanceLevel="11">
          <BriefDescription>A user-assigned name for the core.</BriefDescription>
          <DetailedDescription>
            A user-assigned name for the core.
            The name needs not be unique, but unique names are best.
          </DetailedDescription>
        </String>
        <Int Name="lattice size" NumberOfRequiredValues="1" Extensible="true" AdvanceLevel="11">
          <!-- Since SMTK does not support more than one default value we make it one and extensible -->
          <DefaultValue>0</DefaultValue>
        </Int>
        <ModelEntity Name="instance to be deleted" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11">
          <MembershipMask>instance</MembershipMask>
          <BriefDescription>instances which should be deleted from current assembly</BriefDescription>
          <DetailedDescription>
            Instances which should be deleted from current assembly.
          </DetailedDescription>
        </ModelEntity>
        <ModelEntity Name="instance to be added" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11">
          <MembershipMask>instance</MembershipMask>
          <BriefDescription>instances which should be added into current assembly</BriefDescription>
          <DetailedDescription>
            Instances which should be added into current assembly.
          </DetailedDescription>
        </ModelEntity>
        <String Name="geometry type" Label="geometry type" Version="0" AdvanceLevel="11" NumberOfRequiredValues="1">
          <ChildrenDefinitions>
            <Double Name="duct thickness X" NumberOfRequiredValues="1">
              <BriefDescription>Thickness of the associated rectilinear duct along x axis</BriefDescription>
              <DetailedDescription>
                Thickness of the associated rectilinear duct along x axis.
              </DetailedDescription>
              <DefaultValue>10.0</DefaultValue>
            </Double>
            <Double Name="duct thickness Y" NumberOfRequiredValues="1">
              <BriefDescription>Thickness of the associated rectilinear assembly along y axis</BriefDescription>
              <DetailedDescription>
                Thickness of the associated rectilinear assembly along y axis.
              </DetailedDescription>
              <DefaultValue>10.0</DefaultValue>
            </Double>
            <Double Name="duct thickness" NumberOfRequiredValues="1">
              <BriefDescription>Thickness of the hex duct</BriefDescription>
              <DetailedDescription>
                Thickness of the hex duct.
              </DetailedDescription>
              <DefaultValue>10.0</DefaultValue>
            </Double>
            <Double Name="height" NumberOfRequiredValues="1">
              <BriefDescription>Height of the nuclear core</BriefDescription>
              <DetailedDescription>
                Height of the nuclear core.
              </DetailedDescription>
              <DefaultValue>10.0</DefaultValue>
            </Double>
            <Double Name="z origin" NumberOfRequiredValues="1" AdvanceLevel="11">
              <BriefDescription>z origin of the assembly</BriefDescription>
              <DetailedDescription>
              </DetailedDescription>
                Z origin of the assembly.
              <DefaultValue>0.0</DefaultValue>
            </Double>
          </ChildrenDefinitions>

          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum= "Hex">Hex</Value>
              <Items>
                <Item>duct thickness</Item>
                <Item>height</Item>
                <Item>z origin</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum= "Rect">Rect</Value>
              <Items>
                <Item>duct thickness X</Item>
                <Item>duct thickness Y</Item>
                <Item>height</Item>
                <Item>z origin</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(edit core)" BaseType="result">
      <ItemDefinitions>
          <Void Name="force camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkRGGEditCoreView ...)
      -->
    <View Type="smtkRGGEditCoreView" Title="Edit Core"  FilterByCategory="false"  FilterByAdvanceLevel="false" UseSelectionManager="false">
      <Description>
        Change a nulcear core's properties and layout.
      </Description>
      <AttributeTypes>
        <Att Type="edit core"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeSystem>
