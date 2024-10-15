<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the task-system "AddDependency" Operation -->
<SMTK_AttributeResource Version="7">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="connect ports" BaseType="operation">
      <BriefDescription>
        Connect the output port of one task to the input port of another task.
      </BriefDescription>
      <DetailedDescription>
        This operation checks that the referenced ports have compatible
        configuration-data types before allowing the operation to proceed.
      </DetailedDescription>

      <AssociationsDef Name="from" NumberOfRequiredValues="1" Extensible="true" OnlyResources="false">
        <BriefDescription>The output port to serve as the upstream source of configuration data.</BriefDescription>
        <DetailedDescription>
          This may be an output port or, if the downstream port accepts data of type ObjectsInRoles,
          any persistent object. If a persistent object that is not a port is chosen, then its
          role will be set to "unassigned" and it is up to the downstream port to determine how
          to use the object.
        </DetailedDescription>
        <Accepts>
          <!-- Accept anything (any resource or component). -->
          <Resource Name="smtk::project::Project" Filter="smtk::task::Port"/>
          <Resource Name="smtk::resource::Resource" Filter=""/>
          <Resource Name="smtk::resource::Resource" Filter="*"/>
        </Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <Component Name="to" LockType="Write" NumberOfRequiredValues="1" Extensible="true">
          <Accepts><Resource Name="smtk::project::Project" Filter="smtk::task::Port"/></Accepts>
          <BriefDescription>
            The input port to serve as the downstream consumer of configuration data.
          </BriefDescription>
        </Component>
      </ItemDefinitions>

    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Hints.xml"/>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(connect ports)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
