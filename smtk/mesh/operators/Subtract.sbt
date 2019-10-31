<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "Subtract" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="subtract" BaseType="operation" Label="Mesh - Subtract">
      <AssociationsDef Name="minuend" Label = "Minuend"
                       NumberOfRequiredValues="1" Extensible="false">
        <Accepts><Resource Name="smtk::mesh::Resource" Filter="meshset"/></Accepts>
          <BriefDescription>
            The meshset from which meshsets are subtracted.
          </BriefDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <Component Name="subtrahend" Label = "Subtrahend"
                   NumberOfRequiredValues="1" Extensible="true" >
          <Accepts><Resource Name="smtk::mesh::Resource" Filter="meshset"/></Accepts>
          <BriefDescription>
            The meshsets being subtracted.
          </BriefDescription>
        </Component>
      </ItemDefinitions>
      <BriefDescription>
        Subtract meshsets from a meshset.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Subtract meshsets from a meshset.
        &lt;p&gt;This operator performs a set subtraction on the
        components of meshsets.
      </DetailedDescription>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(subtract)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
