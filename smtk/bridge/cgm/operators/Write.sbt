<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "Write" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="write" BaseType="operator">
      <AssociationsDef Name="Workpiece(s)" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="OpenCascade Boundary Representation (*.brep *.occ);;All files (*.*)">
        </File>
        <String Name="filetype" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(write)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeSystem>
