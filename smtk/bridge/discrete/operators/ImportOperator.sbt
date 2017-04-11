<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Builder" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="import" BaseType="operator">
      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Supported Discrete Formats (*.vtk *.2dm *.3dm *.stl *.sol *.tin *.obj *.map *.poly *.smesh *.shp);;Legacy VTK files (*.vtk);;Solids (*.2dm *.3dm *.stl *.sol *.tin *.obj);;Map files (*.map);;Poly files (*.poly *.smesh);;Shape files (*.shp);;All files (*.*)">
        </File>

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
    <AttDef Type="result(import)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="mesh_created" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
