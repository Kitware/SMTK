<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Polygon "Delete" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="delete" BaseType="operation" Label="Model Entities - Delete">
      <BriefDescription>Delete model entities.</BriefDescription>
      <DetailedDescription>
        Permanently remove model entities (vertices, edges, faces) from a model.
      </DetailedDescription>
      <AssociationsDef Name="entities" NumberOfRequiredValues="1" Extensible="yes">
        <Accepts><Resource Name="smtk::bridge::polygon::Resource" Filter="cell|aux_geom|instance"/></Accepts>
        <BriefDescription>Model entities to delete.</BriefDescription>
        <DetailedDescription>
          Permanently delete all of these entities (and optionally all of
          the higher-dimensional and lower-dimensional entities they bound).
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <Void
          Name="delete higher-dimensional neighbors"
          Label="delete things these entities bound (higher-dimensional neighbors)"
          Optional="true">
          <BriefDescription>Should all bounding entities also be deleted?</BriefDescription>
          <DetailedDescription>
            When disabled (the default), if any associated model entity is
            related to a higher-dimensional model entity (i.e., as a part of its boundary),
            then the operation will fail.

            When enabled, all higher-dimensional model entities bounded
            by any of the associated model entities will also be deleted.
            Thus, deleting a vertex will also delete any edges and faces attached to it.
          </DetailedDescription>
        </Void>
        <Void
          Name="delete lower-dimensional neighbors"
          Label="delete unused boundaries of these entities (lower-dimensional neighbors)"
          Optional="true">
          <BriefDescription>Should all boundary entities also be deleted?</BriefDescription>
          <DetailedDescription>
            When disabled (the default), associated model entity's lower-dimensional model
            entities will not be deleted (i.e., as face's edges and vertices).

            When enabled, all associated model entities'
            lower-dimensional model entities will also be deleted.
            Thus, deleting a face will also delete any edges and
            vertices attached to it if they are not used by other cells.
          </DetailedDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(delete)" BaseType="result">
      <ItemDefinitions>
        <!-- The expunged entities are returned in the base result's "expunged" item. -->
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
