<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">

  <Definitions>

    <!-- ==================== Geometric parameters ========================= -->
    <AttDef Type="source-term" Abstract="true">

      <AssociationsDef Name="source point">
        <Accepts><Resource Name="smtk::session::oscillator::Resource" Filter="aux_geom|2"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <Double Name="omega" NumberOfRequiredValues="1">
          <DefaultValue>0.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">-3.14159265358979323854</Min>
            <Max Inclusive="true">3.14159265358979323854</Max>
          </RangeInfo>
          <Description>Phase of the oscillations</Description>
        </Double>
      </ItemDefinitions>

    </AttDef>

    <AttDef Type="periodic-source" BaseType="source-term">
      <BriefDescription>
        A non-decaying periodic oscillator with phase omega.
      </BriefDescription>
    </AttDef>

    <AttDef Type="decaying-source" BaseType="source-term">
      <BriefDescription>
        A decaying periodic oscillator with phase omega.
      </BriefDescription>
    </AttDef>

    <AttDef Type="damped-source" BaseType="source-term">
      <BriefDescription>
        A damped periodic oscillator with damping factor zeta and phase omega.
      </BriefDescription>

      <ItemDefinitions>
        <Double Name="zeta" NumberOfRequiredValues="1">
          <RangeInfo>
            <Min Inclusive="false">0</Min>
            <Max Inclusive="true">1</Max>
          </RangeInfo>
          <DefaultValue>0.1</DefaultValue>
          <Description>Damping factor</Description>
        </Double>
      </ItemDefinitions>

    </AttDef>

    <!-- =================== Simulation parameters ========================= -->
    <AttDef Type="solver">

      <ItemDefinitions>

        <Int Name="resolution" NumberOfRequiredValues="3">
          <Description>Number of mesh cells along each axis.</Description>
          <DefaultValue>64,64,64</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1</Min>
          </RangeInfo>
        </Int>

        <Int Name="job size" NumberOfRequiredValues="1">
          <Description>Number of parallel ranks to run.</Description>
          <DefaultValue>2</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">1</Min>
          </RangeInfo>
        </Int>

        <Double Name="time step" NumberOfRequiredValues="1">
          <Description>How far the simulation should advance time with each iteration.</Description>
          <DefaultValue>1.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
        </Double>

        <Double Name="end time" NumberOfRequiredValues="1">
          <Description>The time at which the simulation should stop.</Description>
          <DefaultValue>10.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
        </Double>

        <Int Name="ghost levels" NumberOfRequiredValues="1">
          <Description>
            How many additional cell layers should be included
            on shared boundaries of each rank's data.
          </Description>
          <DefaultValue>1</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0</Min>
          </RangeInfo>
        </Int>

        <Void Name="sync" Optional="true" IsEnabledByDefault="false" AdvanceLevel="1">
          <Description>
            When enabled, a barrier will be added at each timestep to
            synchronize the simulation across all ranks.
          </Description>
        </Void>

      </ItemDefinitions>

    </AttDef>

    <!-- ================== Coprocessing parameters ======================== -->
    <AttDef Type="coprocessing parameters">

      <ItemDefinitions>

        <File Name="script" NumberOfRequiredValues="1" ShouldExist="true"
          Optional="true" EnabledByDefault="true" Version="0">
          <BriefDescription>A ParaView/Catalyst script to run</BriefDescription>
        </File>

        <Void Name="connect live" Optional="true" IsEnabledByDefault="true">
          <BriefDescription>
            Should the simulation attempt ParaView Live connections at each timestep?
          </BriefDescription>
        </Void>

        <Void Name="log" Optional="true" IsEnabledByDefault="false">
          <Description>
            When enabled, computation timings and memory consumption will be
            logged to CSV files.
          </Description>
        </Void>

      </ItemDefinitions>

    </AttDef>

  </Definitions>

  <Views>

    <View Type="Group" Title="simulation" TopLevel="true">
      <Views>
        <View Title="solver"/>
        <View Title="source terms"/>
        <View Title="coprocessing"/>
      </Views>
    </View>

    <View Type="ModelEntity" Title="source terms" ModelEntityFilter="aux_geom|2">
      <AttributeTypes>
        <Att Type="source-term" />
      </AttributeTypes>
    </View>

    <View Type="Instanced" Title="solver" Label="solver">
      <InstancedAttributes>
        <Att Name="solver parameters" Type="solver"/>
      </InstancedAttributes>
    </View>

    <View Type="Instanced" Title="coprocessing" Label="coprocessing">
      <InstancedAttributes>
        <Att Name="coprocessing" Type="coprocessing"/>
      </InstancedAttributes>
    </View>

  </Views>

</SMTK_AttributeResource>
