<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Read" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="read" BaseType="operator" Label="Model - Read">
      <BriefDescription>
        Read a cmb file and import it into Conceptual Model Builder.
      </BriefDescription>
      <DetailedDescription>
        Read a cmb file and import it into Conceptual Model Builder.

        This operator only supports read .cmb file.
      </DetailedDescription>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Conceptual Model Builder (*.cmb);;All files (*.*)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(read)" BaseType="result">
      <ItemDefinitions>
        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
