<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the mesh "ExtractByDihedralAngle" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="extract by dihedral angle" BaseType="operation"
            Label="Mesh - Extract Subset by Dihedral Angle">
      <AssociationsDef Name="mesh" LockType="Write" NumberOfRequiredValues="1" Extensible="false">
        <Accepts><Resource Name="smtk::mesh::Resource" Filter="meshset"/></Accepts>
      </AssociationsDef>
      <BriefDescription>
        Extract mesh subset by accepting all neighboring facets that
        meet the selected meshset with a dihedral angle less than a
        given value.
      </BriefDescription>
      <ItemDefinitions>
        <Double Name="dihedral angle" Label="Dihedral Angle"
                Units="degrees" NumberOfRequiredValues="1">
          <BriefDescription>
            The maximum accepted angle (in degrees) between the normals of two adjacent faces.
          </BriefDescription>
          <DefaultValue>30.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0.</Min>
            <Max Inclusive="true">180.</Max>
          </RangeInfo>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(extract by dihedral angle)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
