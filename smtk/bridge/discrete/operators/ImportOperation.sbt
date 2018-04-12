<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Builder" Operation -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="import" BaseType="operation" Label="Model - Import">
      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Legacy VTK files (*.vtk);;Solids (*.2dm *.3dm *.stl *.sol *.tin *.obj);;Map files (*.map);;Poly files (*.poly *.smesh);;Shape files (*.shp)">
        </File>

        <Resource Name="resource" Label="Import into" Optional="true" IsEnabledByDefault="false">
          <Accepts>
            <Resource Name="smtk::bridge::discrete::Resource"/>
          </Accepts>
          <ChildrenDefinitions>
            <String Name="session only" Label="session" Advanced="1">
              <DiscreteInfo DefaultIndex="0">
                <Structure>
                  <Value Enum="this file">import into this file </Value>
                </Structure>
                <Structure>
                  <Value Enum="this session">import into a new file using this file's session</Value>
                </Structure>
              </DiscreteInfo>
            </String>
          </ChildrenDefinitions>
        </Resource>

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
        <Resource Name="resource">
          <Accepts>
            <Resource Name="smtk::bridge::discrete::Resource"/>
          </Accepts>
        </Resource>

        <Component Name="model">
          <Accepts>
            <Resource Name="smtk::bridge::discrete::Resource" Filter=""/>
          </Accepts>
        </Component>

        <Component Name="mesh_created" NumberOfRequiredValues="1">
          <Accepts><Resource Name="smtk::bridge::discrete::Resource" Filter=""/></Accepts>
        </Component>
        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
