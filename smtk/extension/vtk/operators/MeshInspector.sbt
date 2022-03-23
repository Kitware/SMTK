<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the geometric "mesh inspector" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="smtk::geometry::MeshInspector" Label="Mesh Inspector" BaseType="operation">
      <BriefDescription>Inspect the interior of a volumetric mesh with a crinkle-slice.</BriefDescription>
      <DetailedDescription>
        This tool simply displays the selected components' geometry crinkle-sliced by
        a plane that can be adjusted interactively.
      </DetailedDescription>
      <AssociationsDef Name="source" Label="Inputs" NumberOfRequiredValues="1" Extensible="yes">
        <BriefDescription>The volume mesh(es) to inspect.

Space key toggles component for slicing.
Return key toggles unsliced component visibility.</BriefDescription>
        <Accepts>
          <Resource Name="smtk::model::Resource" Filter="volume|face"/>
          <Resource Name="smtk::geometry::Resource" Filter="*"/>
          <Resource Name="smtk::graph::Resource" Filter="*"/>
        </Accepts>
      </AssociationsDef>

      <ItemDefinitions>

        <Group Name="slice_plane" Label="Slice Plane" NumberOfGroups="1">
          <ItemDefinitions>
            <Double Name="origin" Label="Origin" NumberOfRequiredValues="3">
              <BriefDescription>Coordinates of a point on the plane used to slice the geometry.</BriefDescription>
              <DefaultValue>0, 0, 0</DefaultValue>
              <ComponentLabels>
                <Label>X</Label>
                <Label>Y</Label>
                <Label>Z</Label>
              </ComponentLabels>
            </Double>
            <Double Name="normal" Label="Normal" NumberOfRequiredValues="3">
              <BriefDescription>Normal of the plane used to slice the geometry.</BriefDescription>
              <DefaultValue>0, 0, 1</DefaultValue>
              <ComponentLabels>
                <Label>X</Label>
                <Label>Y</Label>
                <Label>Z</Label>
              </ComponentLabels>
            </Double>
          </ItemDefinitions>
        </Group>
        <!-- Dummy input parameter not shown to user, so that ableToOperate() returns false. -->
        <String Name="ensure gui" NumberOfRequiredValues="1" AdvanceLevel="1" />
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(smtk::geometry::MeshInspector)" BaseType="result"/>
  </Definitions>
  <Views>
    <View Type="smtkMeshInspectorView" Title="Mesh Inspector"
      FilterByAdvanceLevel="false" FilterByCategoryMode="false"
      FilterByCategory="false" IgnoreCategories="true">
      <AttributeTypes>
        <Att Type="smtk::geometry::MeshInspector" Name="Mesh inspector">
          <ItemViews>
            <View Path="/source" Type="qtReferenceTree"
              DrawSubtitle="false"
              VisibilityMode="true"
              TextVerticalPad="6"
              TitleFontWeight="1"
              HighlightOnHover="false"
              >
              <PhraseModel Type="smtk::view::ResourcePhraseModel">
                <SubphraseGenerator Type="smtk::view::SubphraseGenerator"/>
                <Badges>
                  <Badge
                    Type="smtk::extension::qt::MembershipBadge"
                    MembershipCriteria="ComponentsWithGeometry"
                    Filter="any"
                    Default="false"/>
                  <Badge
                    Type="smtk::extension::paraview::appcomponents::VisibilityBadge"
                    Default="false"/>
                </Badges>
              </PhraseModel>
            </View>
            <View Path="/slice_plane"
              Type="Slice"
              Style="Crinkle"
              Origin="origin"
              Normal="normal"
              ShowControls="true"/>
          </ItemViews>
        </Att>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
