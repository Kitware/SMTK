<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Builder" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="import" BaseType="operation" Label="Model - Import">

      <!-- Import operations can import a file into an existing
           resource (or an existing resource's session) if one is
           provided. Otherwise, a new resource is created -->
      <AssociationsDef Name="import into" NumberOfRequiredValues="0" Extensible="true" MaxNumberOfValues="1">
        <Accepts><Resource Name="smtk::session::discrete::Resource"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Legacy VTK files (*.vtk);;Solids (*.2dm *.3dm *.stl *.sol *.tin *.obj);;Map files (*.map);;Poly files (*.poly *.smesh);;Shape files (*.shp)">
        </File>

      <!-- In the event that we are importing into an existing
           resource, this enumeration allows the user to select
           whether the import should simply use the resource's session
           or if the imported model should be a part of the resource
           itself -->
        <String Name="session only" Label="session" AdvanceLevel="1">
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="this file">import into this file</Value>
            </Structure>
            <Structure>
              <Value Enum="this session">import into a new file using this file's session</Value>
            </Structure>
          </DiscreteInfo>
        </String>

        <String Name="ShapeBoundaryStyle" Label="Specify Shape File Boundary" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1" Optional="true">
          <BriefDescription>This is required for shape file </BriefDescription>
          <ChildrenDefinitions>
            <String Name="relative margin" Label="As fraction of data diameter" NumberOfRequiredValues="1">
              <DefaultValue>5</DefaultValue>
            </String>
            <String Name="absolute margin" Label="Absolute margin" NumberOfRequiredValues="1">
              <DefaultValue>1</DefaultValue>
              <BriefDescription>all or left+right, bottom+top or left, right, bottom, top</BriefDescription>
            </String>
            <String Name="absolute bounds" Label="Bounding box coordinates" NumberOfRequiredValues="1" >
              <BriefDescription>left, right, bottom, top</BriefDescription>
            </String>
            <File Name="imported polygon" Label="Bounding polyline file" NumberOfRequiredValues="1"
              ShouldExist="true"
              FileFilters="Shape files (*.shp)">
            </File>
          </ChildrenDefinitions>

          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="Import File As-is">None</Value>
            </Structure>
            <Structure>
              <Value Enum="Set Relative Margin">Relative Margin</Value>
              <Items>
                <Item>relative margin</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Set Absolute Margin">Absolute Margin</Value>
              <Items>
                <Item>absolute margin</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Set Bounding Box">Bounding Box</Value>
              <Items>
                <Item>absolute bounds</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Set Bounding File">Bounding File</Value>
              <Items>
                <Item>imported polygon</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>

        <!-- The model imported from the file. -->
        <Resource Name="resource"  Extensible="1" HoldReference="true">
          <Accepts>
            <Resource Name="smtk::session::discrete::Resource"/>
            <Resource Name="smtk::mesh::Resource"/>
          </Accepts>
        </Resource>

        <Component Name="model">
          <Accepts>
            <Resource Name="smtk::session::discrete::Resource" Filter=""/>
          </Accepts>
        </Component>

        <Component Name="mesh_created" NumberOfRequiredValues="1">
          <Accepts><Resource Name="smtk::session::discrete::Resource" Filter=""/></Accepts>
        </Component>
        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
