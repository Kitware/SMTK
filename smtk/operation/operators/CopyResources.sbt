<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "CopyResources" Operation -->
<SMTK_AttributeResource Version="3">

  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="copy resources" BaseType="operation">
      <BriefDescription>
        Create in-memory copies of one or more SMTK resources.
      </BriefDescription>
      <DetailedDescription>
        Copying resources is different than "Save As…" (which
        simply saves the existing resource to a new location);
        the resource and its components are given new unique IDs
        so both they and their originals can be loaded into the
        same process at the same time.

        The new resources do not have their locations set, so
        before exiting, you will be prompted to provide a
        location for each one.
      </DetailedDescription>
      <AssociationsDef LockType="Read" OnlyResources="true" NumberOfRequiredValues="1" Extensible="true">
        <Accepts><Resource Name="smtk::resource::Resource"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(copy resources)" BaseType="result"/>
  </Definitions>

</SMTK_AttributeResource>
