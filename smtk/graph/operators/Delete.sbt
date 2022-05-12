<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "delete" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="delete" Label="delete objects" BaseType="operation">

      <AssociationsDef Name="nodes" NumberOfRequiredValues="1" Extensible="true" HoldReference="true">
        <Accepts><Resource Name="smtk::graph::ResourceBase" Filter="*"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <Void Name="delete dependents" Optional="true" IsEnabledByDefault="false" AdvanceLevel="0">
          <BriefDescription>
            Should dependent components be deleted, or should the operation fail if there are dependents?
          </BriefDescription>
          <DetailedDescription>
            When set, any dependent components will be deleted rather than causing the operation to fail.
            For example, if original geometry is queued for deletion but selections exist on that geometry,
            the operation will fail unless "delete dependents" is enabled. If it is enabled, then more
            components than were requested may be deleted.
          </DetailedDescription>
        </Void>

        <!-- CellSelection uses the "resource" item below to ensure the resource is not
          deleted in a separate thread before the operation runs (which can happen in
          tests).
          -->
        <Resource Name="resource"
          Extensible="true" HoldReference="true"
          NumberOfRequiredValues="0" AdvanceLevel="11"/>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(delete)" BaseType="result">
      <ItemDefinitions>
        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
    <View Type="Operation" Title="delete objects" TopLevel="true" UseSelectionManager="true">
      <InstancedAttributes>
        <Att Type="delete">
          <ItemViews>
            <View Path="/entities" Type="qtReferenceTree">
              <PhraseModel Type="smtk::view::ResourcePhraseModel">
                <SubphraseGenerator Type="smtk::view::SubphraseGenerator"/>
                <Badges>
                  <Badge
                    Type="smtk::extension::qt::MembershipBadge"
                    MembershipCriteria="Components"
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
