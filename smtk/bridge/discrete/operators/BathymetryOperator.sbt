<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "edit bathymetry" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="edit bathymetry" BaseType="operator">
      <ItemDefinitions>
        <Void Name="refetchfromserver" Label="Refetch from server" Version="0" AdvanceLevel="11"
              NumberOfRequiredValues="1"
              Optional="true" IsEnabledByDefault="true" />

        <ModelEntity Name="model" NumberOfRequiredValues="1">
          <MembershipMask>model</MembershipMask>
        </ModelEntity>

        <String Name="operation" Label="Operation" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">

          <ChildrenDefinitions>

            <String Name="bathymetrysource" Label="Loaded Source:" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <ChildrenDefinitions>
                <File Name="bathymetryfile" Label="Load New File:" Version="0"  NumberOfRequiredValues="1"
                       ShouldExist="true"
                       FileFilters="LIDAR (*.pts *.bin *.bin.pts);;LAS (*.las);;DEM (*.dem);;VTK files (*.vtk *.vtp);;All (*.*)">
                </File>
              </ChildrenDefinitions>
              <DiscreteInfo DefaultIndex="0">
                <Structure>
                  <Value Enum="New ...">NEW</Value>
                  <Items>
                    <Item>bathymetryfile</Item>
                  </Items>
                </Structure>
              </DiscreteInfo>
            </String>

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
<!--                <Item>bathymetryfile</Item>  -->
                <Item>bathymetrysource</Item>
                <Item>averaging elevation radius</Item>
                <Item>set highest elevation</Item>
                <Item>set lowest elevation</Item>
                <Item>applyonlytovisiblemesh</Item>
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
