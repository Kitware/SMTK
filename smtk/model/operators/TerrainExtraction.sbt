<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "TerrainExtraction" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="terrain extraction" Label="Model - Terrain Extraction" BaseType="operator">
      <AssociationsDef Name="Point Cloud" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>aux_geom</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Extract terrain from a point cloud.
      </BriefDescription>
      <DetailedDescription>
        Extract terrain from a point cloud.

      </DetailedDescription>
:   </AttDef>
    <!-- Result -->
    <AttDef Type="result(assign colors)" BaseType="result"/>
  </Definitions>

  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkTerrainExtractionView ...)
      -->
    <View Type="smtkTerrainExtractionView" Title="Terrain Extraction">
      <Description>
        Extract terrain from a point cloud.
      </Description>
      <AttributeTypes>
        <Att Type="terrain extraction"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeSystem>
