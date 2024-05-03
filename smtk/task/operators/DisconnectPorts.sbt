<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the task-system "AddDependency" Operation -->
<SMTK_AttributeResource Version="7">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="disconnect ports" BaseType="operation">
      <BriefDescription>
        Disconnects the output port of one task from a persistent object.
      </BriefDescription>
      <DetailedDescription>
        This operation checks that there is a connection before allowing the operation to proceed.
      </DetailedDescription>

      <ItemDefinitions>
        <String Name="arc type">
          <BriefDescription>
            The arc type. This value is fixed to be "task dependency;" you may
            not change it as this operation supports only one type of arc removal.
          </BriefDescription>
          <ChildrenDefinitions>
            <Group Name="port endpoints" NumberOfRequiredGroups="0" Extensible="true">
              <ItemDefinitions>

                <Reference Name="from" LockType="Write" NumberOfRequiredValues="1">
                  <Accepts>
                    <Resource Name="smtk::resource::Resource" Filter=""/>
                    <Resource Name="smtk::resource::Resource" Filter="*"/>
                  </Accepts>
                  <BriefDescription>
                    An output port or other persistent object that serves as the upstream source of configuration data.
                  </BriefDescription>
                  <DetailedDescription>
                    The source of configuration data for an input port.

                    This may be an output port that produces compatible data or any
                    persistent object of a type accepted by the "to" port.
                  </DetailedDescription>
                </Reference>

                <Component Name="to" LockType="Write" NumberOfRequiredValues="1">
                  <Accepts><Resource Name="smtk::project::Project" Filter="smtk::task::Port"/></Accepts>
                  <BriefDescription>
                    An input port that consumes configuration data from upstream.
                  </BriefDescription>
                </Component>

              </ItemDefinitions>
            </Group>
          </ChildrenDefinitions>

          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="port connection">port connection</Value>
              <Items>
                <Item>port endpoints</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>


    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Hints.xml"/>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(disconnect ports)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
