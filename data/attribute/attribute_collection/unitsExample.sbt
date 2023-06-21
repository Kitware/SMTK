<?xml version="1.0"?>
<SMTK_AttributeResource Version="6">
  <Definitions>
    <AttDef Type="UnitsExample">
      <ItemDefinitions>
        <Double Name="No Units 1">
            <BriefDescription>No Units, No Default</BriefDescription>
        </Double>
        <Double Name="No Units 2">
            <BriefDescription>No Units, Default Value</BriefDescription>
            <DefaultValue>3.14159</DefaultValue>
        </Double>

        <Double Name="Length" Units="m"><DefaultValue>5.5</DefaultValue></Double>

        <Double Name="Distance" Units="ft" />

        <Double Name="Speed" Units="meter-per-sec">
            <BriefDescription>Unrecognized Units</BriefDescription>
        </Double>
      </ItemDefinitions>
    </AttDef>
   </Definitions>
  <Views>
    <View Type="Instanced" Title="Units Example" TopLevel="true">
      <InstancedAttributes>
        <Att Type="UnitsExample" Name="UnitsExample" />
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
