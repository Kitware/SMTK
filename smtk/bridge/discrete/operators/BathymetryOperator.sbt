<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "edit bathymetry" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="edit bathymetry" BaseType="operator">
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1">
          <MembershipMask>model</MembershipMask>
        </ModelEntity>

        <String Name="operation" Label="Operation" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">

          <ChildrenDefinitions>
           <File Name="bathymetryfile" Label="Load File:" Version="0"  NumberOfRequiredValues="1"
                   ShouldExist="true"
                   FileFilters="Supported files (*.pts *.bin *.bin.pts *.las *.dem *.vtk *.vtp *.vti *.2dm *.3dm *.obj *.tin *.poly *.smesh *.fac);;LIDAR (*.pts *.bin *.bin.pts);;LAS (*.las);;DEM (*.dem);;VTK files (*.vtk *.vtp *.vti);;CMB Geometry files(*.2dm *.3dm *.obj *.tin *.poly *.smesh *.fac);;All (*.*)">
            </File>

            <Double Name="averaging elevation radius" Label="Radius for Averaging Elevation:" Version="0" NumberOfRequiredValues="1">
              <DefaultValue>1.0</DefaultValue>
            </Double>

            <Double Name="set highest elevation" Label="Set Highest Elevation:" Version="0" NumberOfRequiredValues="1" Optional="true">
              <DefaultValue>0.0</DefaultValue>
            </Double>

            <Double Name="set lowest elevation" Label="Set Lowest Elevation:" Version="0" NumberOfRequiredValues="1" Optional="true">
              <DefaultValue>0.0</DefaultValue>
            </Double>

            <Void Name="applyonlytovisiblemesh" Label="Apply only to the visible meshes on the model" Version="0" AdvanceLevel="0" NumberOfRequiredValues="0"
            Optional="true" IsEnabledByDefault="false">
            </Void>

          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="Apply Bathymetry">Apply Bathymetry</Value>
              <Items>
                <Item>bathymetryfile</Item>
                <Item>averaging elevation radius</Item>
                <Item>set highest elevation</Item>
                <Item>set lowest elevation</Item>
<!--                <Item>applyonlytovisiblemesh</Item>  -->
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Remove Bathymetry">Remove Bathymetry</Value>
            </Structure>
          </DiscreteInfo>

        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(edit bathymetry)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
