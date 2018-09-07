<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "Write" Operation -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operation -->
    <AttDef Type="write" BaseType="operation">
      <AssociationsDef Name="Workpiece(s)" NumberOfRequiredValues="1" Extensible="true">
        <Accepts><Resource Name="smtk::session::cgm::Resource" Filter="model"/></Accepts>
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
