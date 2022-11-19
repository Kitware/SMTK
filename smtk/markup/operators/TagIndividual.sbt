<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the markup "tag individual" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="tag individual" Label="tag with ontology id" BaseType="operation">

      <BriefDescription>Mark a component as being a named individual of an ontology class identifier.</BriefDescription>
      <DetailedDescription>
        Mark components as being named individuals of an ontology class identifier.
        All associated target objects are connected with arcs to the ontology identifier, which
        is created as needed along with the ontology.
      </DetailedDescription>
      <AssociationsDef Name="targets" NumberOfRequiredValues="1" Extensible="true">
        <BriefDescription>The input components to be tagged.</BriefDescription>
        <Accepts><Resource Name="smtk::markup::Resource" Filter="*"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>

        <String Name="ontology" NumberOfRequiredValues="1">
          <BriefDescription>A name for the ontology node to use or create.</BriefDescription>
        </String>

        <String Name="name" NumberOfRequiredValues="1">
          <BriefDescription>A name for the ontology identifier node to use or create.</BriefDescription>
        </String>

        <String Name="url" NumberOfRequiredValues="1">
          <BriefDescription>The canonical URL for the ontology class identifier.</BriefDescription>
        </String>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(tag individual)" BaseType="result">
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Operation" Title="tag individuals" TopLevel="true" UseSelectionManager="true">
      <InstancedAttributes>
        <Att Type="tag individual">
          <ItemViews>
            <View Path="/targets" Type="qtReferenceTree">
              <PhraseModel Type="smtk::view::ResourcePhraseModel">
                <SubphraseGenerator Type="smtk::view::SubphraseGenerator"/>
                <Badges>
                  <Badge
                    Type="smtk::extension::qt::MembershipBadge"
                    MembershipCriteria="Components"
                    Filter="*"
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
