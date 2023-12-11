<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the geometric "ExportFaceSet" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="smtk::geometry::ExportFaceset" Label="Export Faceset" BaseType="operation">
      <BriefDescription>Export surface model to STL / OBJ / PLY.</BriefDescription>
      <DetailedDescription>
        Export a selected surface / faceset to an STL / OBJ / PLY file format.
      </DetailedDescription>
      <AssociationsDef Name="source" Label="Inputs" NumberOfRequiredValues="1" Extensible="no">
        <Accepts>
          <Resource Name="smtk::model::Resource" Filter="volume|face"/>
          <Resource Name="smtk::geometry::Resource" Filter="*"/>
        </Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
              FileFilters="Stereolithography File (*.stl);;Wavefront OBJ File (*.obj);;Stanford Triangle Files (*.ply)">
          <BriefDescription>The destination file</BriefDescription>
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(smtk::geometry::ExportFaceset)" BaseType="result"/>
  </Definitions>
  <Views>
    <View Type="Operation" Title="Export Faceset"
      FilterByAdvanceLevel="false" FilterByCategoryMode="false"
      FilterByCategory="false" IgnoreCategories="true">
      <InstancedAttributes>
        <Att Type="smtk::geometry::ExportFaceset" Name="Export Faceset">
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
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
