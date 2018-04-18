<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "EditMaterial" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="edit material" Label="Model - Edit Material" BaseType="operator">
      <BriefDescription>Edit material for RGG model.</BriefDescription>
      <DetailedDescription>
        Edit an available material for an RGG model.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>

        <String Name="name" AdvanceLevel="11">
          <BriefDescription>A user assigned name for the material</BriefDescription>
        </String>

        <String Name="label" AdvanceLevel="11">
        </String>
        <Double Name="color" NumberOfRequiredValues="4" AdvanceLevel="11">
          <BriefDescription>
            r, g, b, a. Each one should stay within [0,1] range.
          </BriefDescription>
          <DetailedDescription>
            r, g, b, a. Each one should stay within [0,1] range.
          </DetailedDescription>
        </Double>

        <Double Name="temperature" AdvanceLevel="11">
          <BriefDescription>Material temperature in Kelvin</BriefDescription>
          <RangeInfo><Min Inclusive="true">0</Min></RangeInfo>
        </Double>

        <Double Name="thermalExpansion" AdvanceLevel="11">
          <BriefDescription>Material thermal expansion</BriefDescription>
          <RangeInfo><Min Inclusive="true">0</Min></RangeInfo>
	  <DefaultValue>0.</DefaultValue>
        </Double>

        <Double Name="density" AdvanceLevel="11">
          <BriefDescription>Material density</BriefDescription>
          <RangeInfo><Min Inclusive="true">0</Min></RangeInfo>
        </Double>

        <String Name="densityType" Label="Density Type" AdvanceLevel="11">
          <BriefDescription>Material density in atoms/barn-cm or g/cm^3.</BriefDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="atoms/barn-cm">adensity</Value>
            <Value Enum="g/cm^3">wdensity</Value>
          </DiscreteInfo>
        </String>

        <String Name="compositionType" Label="Composition Type" AdvanceLevel="11">
          <BriefDescription>Description of how material components
          are composed to form the material</BriefDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="weight fractions">wfracs</Value>
            <Value Enum="atom fractions">afracs</Value>
            <Value Enum="atom densities">adens</Value>
            <Value Enum="weight densities">wdens</Value>
          </DiscreteInfo>
        </String>

        <String Name="component" Label="Component"
                Extensible="True" AdvanceLevel="11">
          <BriefDescription>Components that comprise the material</BriefDescription>
        </String>

        <Double Name="content" Extensible="True" AdvanceLevel="11">
          <BriefDescription>Fraction or density associated with a component</BriefDescription>
          <RangeInfo><Min Inclusive="true">0</Min></RangeInfo>
        </Double>

      </ItemDefinitions>

    </AttDef>
    <!-- Result -->
    <AttDef Type="result(edit material)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkRGGEditMaterialView ...)
      -->
    <View Type="smtkRGGEditMaterialView" Title="Edit Material"  FilterByCategory="false"  FilterByAdvanceLevel="false" UseSelectionManager="false">
      <Description>
        Edit a preexisting material definition associated with this
        RGG model.
      </Description>
      <AttributeTypes>
        <Att Type="edit material"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeSystem>
