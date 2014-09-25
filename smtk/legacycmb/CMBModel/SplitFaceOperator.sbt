<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Split Face" Operator -->
<SMTK_AttributeManager Version="1">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="split face" BaseType="operator">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1">
        </ModelEntity>
        <ModelEntity Name="face to split" NumberOfRequiredValues="1">
        </ModelEntity>
        <Double Name="feature angle" NumberOfRequiredValues="1">
          <DefaultValue>15.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0.</Min>
            <Max Inclusive="true">360.</Max>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(split face)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeManager>
