
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="Test">
      <ItemDefinitions>
        <Group Name="opt1" Label="Pick At Least 2"
          IsConditional="true" MinNumberOfChoices="2" MaxNumberOfChoices="0">
          <ItemDefinitions>
            <String Name="opt1"/>
            <Int Name="opt2"/>
            <Double Name="opt3"/>
          </ItemDefinitions>
        </Group>
        <Group Name="opt2" Label="Pick No more than 2"
          IsConditional="true" MinNumberOfChoices="0" MaxNumberOfChoices="2">
          <ItemDefinitions>
            <String Name="opt1"/>
            <Int Name="opt2"/>
            <Double Name="opt3"/>
          </ItemDefinitions>
        </Group>
        <Group Name="opt3" Label="Pick exactly 2"
          IsConditional="true" MinNumberOfChoices="2" MaxNumberOfChoices="2">
          <ItemDefinitions>
            <String Name="opt1"/>
            <Int Name="opt2"/>
            <Double Name="opt3"/>
          </ItemDefinitions>
        </Group>
        <Group Name="opt4" Label="Pick At Least 1 but no more than 2"
          IsConditional="true" MinNumberOfChoices="1" MaxNumberOfChoices="2">
          <ItemDefinitions>
            <String Name="opt1"/>
            <Int Name="opt2"/>
            <Double Name="opt3"/>
          </ItemDefinitions>
        </Group>
        <Group Name="opt5" Label="Pick Any"
          IsConditional="true" MinNumberOfChoices="0" MaxNumberOfChoices="0">
          <ItemDefinitions>
            <String Name="opt1"/>
            <Int Name="opt2"/>
            <Double Name="opt3"/>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Instanced" Title="TopLevel" TopLevel="true" TabPosition="North">
      <InstancedAttributes>
        <Att Type="Test" Name="Test1"/>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
