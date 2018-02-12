<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "TerrainExtraction" Operation -->
<SMTK_AttributeSystem Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="terrain extraction" Label="Model - Terrain Extraction" BaseType="operation">
      <AssociationsDef Name="Point Cloud" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>aux_geom</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Extract a terrain from a point cloud.
      </BriefDescription>
      <DetailedDescription>
        Extract a terrain from a point cloud.
        In advanced mode, user can pick an auxiliary geomtry to view after the
        extraction rather then load the finest result.
        Minimum spacing computation is a simplified version of it's PointsBuilder counterpart.
        ClipPolygons and PointThresholdFilter is not used.
      </DetailedDescription>
      <ItemDefinitions>
        <Void
          Name = "pick custom result"
          Label = "Pick result after extraction"
          Version = "0"
          NumberOfRequiredValues = "1"
          IsEnabledByDefault = "false"
          AdvanceLevel = "1"
          Option = "true"
          Optional = "true">
          <BriefDescription>Allow user to specify an auxiliary geomtry to view after the extraction
           when "View Output Upon Completion" is enabled.
          </BriefDescription>
          <DetailedDescription>Allow user to specify an auxiliary geomtry to view after the extraction
           when "View Output Upon Completion" is enabled. Otherwise the finest result would be loaded in.
          </DetailedDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(terrain extraction)" BaseType="result"/>
  </Definitions>

  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkTerrainExtractionView ...)
      -->
    <View Type="smtkTerrainExtractionView" Title="Terrain Extraction" UseSelectionManager="true">
      <Description>
        Extract a terrain from a point cloud.
        In advanced mode, user can pick an auxiliary geomtry to view after the
        extraction rather then load the finest result.
        Minimum spacing: The minimum space of the grid to generate terrain.
        Minimum spacing computation is a simplified version of it's PointsBuilder counterpart.
        ClipPolygons and PointThresholdFilter is not used.
        Mask Size: At 0, all samples are used thus more robust to noise (but slower), while
        increasing towards 1 tends towards a more even sampling (faster, but may not work well if the data is anisotropic)
      </Description>
      <AttributeTypes>
        <Att Type="terrain extraction"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeSystem>
