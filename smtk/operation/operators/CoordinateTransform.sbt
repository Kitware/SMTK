<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "CoordinateTransform" Operation -->
<SMTK_AttributeResource Version="3">
  <ItemBlocks>
    <Block Name="CoordinateFrameItems">
      <ItemDefinitions>
        <Double Name="origin" Label="origin" NumberOfRequiredValues="3">
          <DefaultValue>0, 0, 0</DefaultValue>
          <BriefDescription>Origin point of the coordinate frame.</BriefDescription>
        </Double>
        <Double Name="x axis" Label="x axis" NumberOfRequiredValues="3">
          <DefaultValue>1, 0, 0</DefaultValue>
          <BriefDescription>XAxis vector of the coordinate frame.</BriefDescription>
        </Double>
        <Double Name="y axis" Label="y axis" NumberOfRequiredValues="3">
          <DefaultValue>0, 1, 0</DefaultValue>
          <BriefDescription>YAxis vector of the coordinate frame.</BriefDescription>
        </Double>
        <Double Name="z axis" Label="z axis" NumberOfRequiredValues="3">
          <DefaultValue>0, 0, 1</DefaultValue>
          <BriefDescription>ZAxis vector of the coordinate frame.</BriefDescription>
        </Double>
        <Reference Name="parent" Label="parent" NumberOfRequiredValues="1" Optional="true" IsEnabledByDefault="false">
          <Accepts><Resource Name="smtk::resource::Resource" Filter="*"/></Accepts>
          <BriefDescription>Reference to the parent component.</BriefDescription>
        </Reference>
      </ItemDefinitions>
    </Block>
  </ItemBlocks>

  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="coordinate transform" BaseType="operation">
      <AssociationsDef Name="source" NumberOfRequiredValues="1" Extensible="true">
        <Accepts><Resource Name="smtk::resource::Resource" Filter="*"/></Accepts>
      </AssociationsDef>
      <BriefDescription>Set (or remove) a coordinate transform on input component(s).</BriefDescription>
      <DetailedDescription>
        Set (or remove) the coordinate-system transform that moves input
        component(s) into world coordinates.
      </DetailedDescription>
      <ItemDefinitions>
        <Group Name="from">
          <ItemDefinitions>
            <Block Name="CoordinateFrameItems"/>
          </ItemDefinitions>
        </Group>
        <Group Name="to">
          <ItemDefinitions>
            <Block Name="CoordinateFrameItems"/>
          </ItemDefinitions>
        </Group>
        <Void Name="remove" Optional="true" IsEnabledByDefault="false">
          <BriefDescription>
            If enabled, indicates that the transform should be removed
            from all inputs (in which case the "from" and "to" items are ignored).
          </BriefDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(coordinate transform)" BaseType="result"/>
  </Definitions>
  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkCoordinateTransformView ...)
      -->
    <View Type="smtkCoordinateTransformView" Title="coordinate transform"
      FilterByCategory="false"  FilterByAdvanceLevel="false" UseSelectionManager="true">
      <Description>
        Provide a "from" and a "to" coordinate frame.
        The associated components will then have their transform set such that
        the "from" origin becomes the "to" origin and the "from" axis vectors are aligned
        with the "to" axes.
      </Description>
      <AttributeTypes>
        <Att Type="coordinate transform" Name="coordinate transform">
          <ItemViews>
            <View Item="from" Type="CoordinateFrame"
              Origin="origin" XAxis="x axis" YAxis="y axis" ZAxis="z axis" Parent="parent" ShowControls="true"/>
            <View Item="to" Type="CoordinateFrame"
              Origin="origin" XAxis="x axis" YAxis="y axis" ZAxis="z axis" Parent="parent" ShowControls="true"/>
          </ItemViews>
        </Att>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
