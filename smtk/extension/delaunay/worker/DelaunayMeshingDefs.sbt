<?xml version="1.0"?>
<SMTK_AttributeSystem Version="2">
<!--**********  Attribute Definitions ***********-->
  <Definitions>
    <AttDef Type="Globals" Label="Global Options" BaseType="" Version="0" Unique="true">
      <ItemDefinitions>

        <!--********** Validation ***********-->
        <Void Name="Validate" Label="Validate Polygons" Version="0" Optional="true" IsEnabledByDefault="false" NumberOfRequiredValues="1">
          <BriefDescription>Validate polygons before discretization</BriefDescription>
        </Void>

      </ItemDefinitions>
    </AttDef>

  </Definitions>
  <!--********** Workflow Views ***********-->
  <Views>
    <View Type="Instanced" Title="Global Meshing Controls">
      <InstancedAttributes>
        <Att Name="Globals" Type="Globals" />
      </InstancedAttributes>
    </View>
    <View Type="Group" Title="Meshing Parameters" TopLevel="true" Style="Tiled">
      <DefaultColor>1., 1., 0.5, 1.</DefaultColor>
      <InvalidColor>1, 0.5, 0.5, 1</InvalidColor>
      <Views>
        <View Title="Global Meshing Controls" />
      </Views>
    </View>
  </Views>
</SMTK_AttributeSystem>
