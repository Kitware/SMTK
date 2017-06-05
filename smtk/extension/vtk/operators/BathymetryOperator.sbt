<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Model "apply bathymetry" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="apply bathymetry" Label="Mesh - Apply Bathymetry" BaseType="operator">
      <BriefDescription>
        Add or Remove elevation from a model or mesh.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Add or Remove elevation from a model or mesh.
        &lt;p&gt;This operator modifies the z-coordinate of a model, a
        mesh or both a model and a mesh according to an auxiliary
        geometry. The elevation data can be applied or removed.
      </DetailedDescription>
      <ItemDefinitions>

        <String Name="operation" Label="Operation" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">

          <ChildrenDefinitions>
            <ModelEntity Name="auxiliary geometry" Label = "auxiliary geometry" NumberOfRequiredValues="1">
              <MembershipMask>aux_geom</MembershipMask>
              <BriefDescription>
              Add auxiliary geometry first then apply bathymetry.
            </BriefDescription>
            </ModelEntity>
            <ModelEntity Name="model" Label = "model" NumberOfRequiredValues="1">
              <MembershipMask>model</MembershipMask>
            </ModelEntity>
            <MeshEntity Name="mesh" NumberOfRequiredValues="0" Extensible = "1">
              <MembershipMask>mesh</MembershipMask>
            </MeshEntity>

            <Double Name="averaging elevation radius" Label="Radius for Averaging Elevation:" Version="0" NumberOfRequiredValues="1">
              <DefaultValue>1.0</DefaultValue>
            </Double>

            <Double Name="set highest elevation" Label="Set Highest Elevation:" Version="0" NumberOfRequiredValues="1" Optional="true">
              <DefaultValue>0.0</DefaultValue>
            </Double>

            <Double Name="set lowest elevation" Label="Set Lowest Elevation:" Version="0" NumberOfRequiredValues="1" Optional="true">
              <DefaultValue>0.0</DefaultValue>
            </Double>

        <Void Name="invert scalars" Label="Invert Scalar Values" Version="0"
              Optional="true" IsEnabledByDefault="false" AdvanceLevel="1">
          <BriefDescription>This toggle adds a prefactor of -1 to the
          values in the auxiliary geometry's bathymetry values.</BriefDescription>
        </Void>

            <Void Name="applyonlytovisiblemesh" Label="Apply only to the visible meshes on the model" Version="0" AdvanceLevel="0" NumberOfRequiredValues="0"
            Optional="true" IsEnabledByDefault="false">
            </Void>

          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="Apply Bathymetry (Auto)">Apply Bathymetry (Auto)</Value>
              <Items>
                <Item>auxiliary geometry</Item>
                <Item>averaging elevation radius</Item>
                <Item>set highest elevation</Item>
                <Item>set lowest elevation</Item>
                <Item>invert scalars</Item>
<!--                <Item>applyonlytovisiblemesh</Item>  -->
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Apply Bathymetry (Model&Mesh)">Apply Bathymetry (Model&Mesh)</Value>
              <Items>
                <Item>auxiliary geometry</Item>
                <Item>averaging elevation radius</Item>
                <Item>set highest elevation</Item>
                <Item>set lowest elevation</Item>
                <Item>invert scalars</Item>
                <Item>mesh</Item>
<!--                <Item>applyonlytovisiblemesh</Item>  -->
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Apply Bathymetry (Model Only)">Apply Bathymetry (Model Only)</Value>
              <Items>
                <Item>auxiliary geometry</Item>
                <Item>averaging elevation radius</Item>
                <Item>set highest elevation</Item>
                <Item>set lowest elevation</Item>
                <Item>invert scalars</Item>
<!--                <Item>applyonlytovisiblemesh</Item>  -->
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Apply Bathymetry (Mesh only)">Apply Bathymetry (Mesh Only)</Value>
              <Items>
                <Item>auxiliary geometry</Item>
                <Item>averaging elevation radius</Item>
                <Item>set highest elevation</Item>
                <Item>set lowest elevation</Item>
                <Item>invert scalars</Item>
                <Item>mesh</Item>
<!--                <Item>applyonlytovisiblemesh</Item>  -->
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Remove Bathymetry">Remove Bathymetry</Value>
              <Items>
                <Item>model</Item>
              </Items>
            </Structure>
          </DiscreteInfo>

        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(apply bathymetry)" BaseType="result">
      <ItemDefinitions>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="1"/>
        <!-- The modified entities are stored in the base result's "modified" item. -->
        <MeshEntity Name="mesh_modified" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
