<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "CreateFace" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="create face" BaseType="operator">
      <ItemDefinitions>
        <Int Name="surface type" NumberOfRequiredValues="1">
          <BriefDescription>The type of surface to create.</BriefDescription>
          <ChildrenDefinitions/>
          <DiscreteInfo DefaultIndex="0">
            <!-- Values from CGM's GeometryType enum in util/GeometryDefines.h -->
            <Structure>
              <Value Enum="plane">12</Value>
            </Structure>
            <Structure>
              <Value Enum="best fit">16</Value>
            </Structure>
          </DiscreteInfo>
        </Int>
        <ModelEntity Name="edges" NumberOfRequiredValues="1" Extensible="true">
          <BriefDescription>One or more pre-existing model edges.</BriefDescription>
        </ModelEntity>
        <Int Name="color" NumberOfRequiredValues="1">
          <BriefDescription>The CGM color index assigned to the face.</BriefDescription>
          <Min Inclusive="true">0</Min>
          <Max Inclusive="true">15</Max>
          <DefaultValue>1</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(create face)" BaseType="result">
      <ItemDefinitions>
        <!-- The face created. -->
        <ModelEntity Name="face" NumberOfRequiredValues="1" MembershipMask="260"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
